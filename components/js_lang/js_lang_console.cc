/* Copyright (c) 2025 Percona LLC and/or its affiliates. All rights
   reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; version 2 of
   the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA */

/*
  This and corresponding header file contain implementation of debugging
  console support for our JS routines.

  Notes on high-level design of debugging console support in JS routines
  ======================================================================

  In general, in our implementation we try to follow Console Standard
  as described at https://console.spec.whatwg.org/ and MDN documentation
  at https://developer.mozilla.org/en-US/docs/Web/API/console,

  At this point we support most of functions which are mentioned in the
  standard. Only support for table(), trace(), dir() and dirxml() is
  missing (they can be invoked but do nothing).

  TODO: We need to improve the way in which objects are logged. We need
        to show their internal structure in the log similarly to Node.JS.

  Our implementation supports Debug, Info, Warning and Error log severity
  levels. This is identical to what is supported by Chrome and Edge
  browsers.

  For obvious reasons, we do not support such interactive features of
  debugging console as collapsed groups or drilling down object structure
  like browsers do. Nor do we support CSS styling of output. In this
  respect we are similar to Node.JS.

  OTOH in addition to giving users access to plain version of console
  output, similar to what Node.JS provides, through JS_GET_CONSOLE_LOG()
  UDF, we also provide access to extended structured representation of
  console output in JSON format through JS_GET_CONSOLE_LOG_JSON() UDF.
  This representation provides additional information (like message
  timestamps) and enables further flexible processing of the console log.
  For exampe, one can convert console output to an SQL table form by
  applying JSON_TABLE() function to this JSON representation.
*/

#include "js_lang_console.h"

#include <iomanip>
#include <sstream>

#include "js_lang_core.h"

void Js_console::write_log(Log_level_type level, std::string &&msg,
                           bool is_group_label) {
  Log_line line(log_line_clock::now(), level, m_group_stack.size(),
                get_group_stack_json(), is_group_label, std::move(msg));

  // We read system variable value once, and then use this value
  // for all checks below to avoid breaking assumptions about buffer
  // emptiness.
  const size_t max_log_size = s_max_log_size;

  if (line.get_approx_size() > max_log_size) {
    /*
      Extreme case. We are asked to log message so huge that it exceeds
      console buffer size limit. We handle this unlikely case by clearing
      the log and adding special dummy message to it instead.
    */
    line.group_level = 0;
    line.group_stack_json = "[]";
    line.message =
        "<The message and part of its context have been suppressed as they "
        "exceed " MAX_CONSOLE_LOG_SIZE_VAR_NAME " limit>";

    m_log.clear();
    m_approx_log_size = line.get_approx_size();
    m_log.push_back(std::move(line));
    return;
  }

  // We need to remove some old lines from the log buffer if adding new
  // message will exceed the size limit. Thanks to the above check we
  // can be sure that this is always possible.
  while (m_approx_log_size + line.get_approx_size() > max_log_size) {
    assert(!m_log.empty());
    m_approx_log_size -= m_log.front().get_approx_size();
    m_log.pop_front();
  }

  m_approx_log_size += line.get_approx_size();
  m_log.push_back(std::move(line));
}

/**
  Build message from the arguments of one of console.log() family of JS calls.

  @param prepend_message  String prepend to the built message (or nullptr).
  @param first_arg        Index of the first argument of JS call to be
                          processed.
  @param args             Object representing arguments of JS call.

  @note This method is similar to a combination of Formatter and Printer
        operations described by Console standard.
        See: https://console.spec.whatwg.org/#formatter

  @return String with message or std::nullopt in case of error. In the latter
          case the JS call is also marked as ending with exception.
*/
static std::optional<std::string> build_message(
    const char *prepend_message, int first_arg,
    const v8::FunctionCallbackInfo<v8::Value> &args) {
  std::string result;
  v8::Isolate *isolate = args.GetIsolate();
  int i = first_arg;

  if (prepend_message != nullptr) result.append(prepend_message);

  // If JS call from console.log() family we are executing has more than
  // one argument and its first argument is JS String, we need to process
  // this string, by replacing format specifiers in it with converted
  // values of later arguments, where conversion procedure for each
  // argument is prescribed by corresponding specifier.
  //
  // So below we will loop through this String from one specifier to
  // another.
  if (args.Length() > (i + 1) && args[i]->IsString()) {
    // As in other places we assume here that conversions of JS string
    // to UTF8 can't fail (as OOM will abort the process).
    v8::String::Utf8Value message(isolate, args[i]);
    const char *chunk_start = *message;
    const char *end = chunk_start + message.length();
    ++i;

    while (chunk_start < end) {
      // We do not need to process specifiers if there are no unprocessed
      // arguments left. So we simply copy the remainder of the first
      // argument in this case.
      if (i >= args.Length()) {
        result.append(chunk_start, end - chunk_start);
        break;
      }

      // Find next format specifier occurrence. Nice that we can use
      // strchr(..., '%') to do this even for UTF-8 string.
      const char *pos;
      pos = strchr(chunk_start, '%');

      // Handle no format specifier case.
      if (pos == nullptr || pos + 1 >= end) {
        result.append(chunk_start, end - chunk_start);
        break;
      }

      // Copy part of the string until the specifier found.
      result.append(chunk_start, pos - chunk_start);

      switch (*(pos + 1)) {
        // %s means that argument should be converted to String.
        case 's': {
          v8::TryCatch try_catch(isolate);
          v8::Local<v8::String> arg_str;
          if (!(args[i]
                    ->ToString(isolate->GetCurrentContext())
                    .ToLocal(&arg_str))) {
            // Conversion to String might fail. We need to rethrow
            // error to caller in this case.
            args.GetReturnValue().Set(try_catch.ReThrow());
            return std::nullopt;
          }

          // Again, we assume that conversion of JS String to UTF-8
          // can't fail (since OOM kills the process).
          v8::String::Utf8Value val(isolate, arg_str);
          result.append(*val, val.length());
          ++i;
          break;
        }
        // %d and %i mean that argument should be converted to integer.
        // Spec says that this should be done by calling JS parseInt(arg, 10)
        // function, but we take much simpler approach instead (for now).
        //
        // The main difference between two is that parseInt() uses JS
        // Number (i.e. floating-point) number internally, hence it can
        // handle bigger numbers than our implementation at the price of
        // precision loss for values bigger than 2^53-1.
        //
        // TODO. Study if it is really necessary and feasible to use
        //       approach suggested by the standard.
        case 'd':
        case 'i': {
          // Like parseInt() we convert argument to String first.
          v8::TryCatch try_catch(isolate);
          v8::Local<v8::String> arg_str;
          if (!(args[i]
                    ->ToString(isolate->GetCurrentContext())
                    .ToLocal(&arg_str))) {
            args.GetReturnValue().Set(try_catch.ReThrow());
            return std::nullopt;
          }
          // Converting JS String to UTF-8 should not fail (unless OOM).
          v8::String::Utf8Value val(isolate, arg_str);

          char *str_end;
          long long arg_i = std::strtoll(*val, &str_end, 10);
          if (arg_i == 0 && str_end == *val) {
            result.append("NaN");
          } else {
            // 25 is more than enough for any 64-bit int value.
            char buff[25];
            snprintf(buff, sizeof(buff), "%lld", arg_i);
            result.append(buff);
          }
          ++i;
          break;
        }
        // %f means that argument should be converted to floating point
        // number. Again console standard says to use parseFloat(arg, 10),
        // but we use a bit simpler approach, instead.
        //
        // Unlike for %i/%d case the main difference between the standard
        // and our approach is probably related to how floating point value
        // is converted to a string.
        //
        // TODO. Study if it is really necessary and feasible to use
        //       approach suggested by the standard.
        case 'f': {
          // Like parseFloat() we convert argument to String first.
          v8::TryCatch try_catch(isolate);
          v8::Local<v8::String> arg_str;
          if (!(args[i]
                    ->ToString(isolate->GetCurrentContext())
                    .ToLocal(&arg_str))) {
            args.GetReturnValue().Set(try_catch.ReThrow());
            return std::nullopt;
          }
          // Converting JS String to UTF-8 should not fail (unless OOM).
          v8::String::Utf8Value val(isolate, arg_str);

          char *str_end;
          double arg_f = std::strtod(*val, &str_end);
          if (arg_f == 0 && str_end == *val) {
            result.append("NaN");
          } else {
            // The question about buffer which is sufficient to print
            // double is non-trivial, so we cheat (for now).
            std::stringstream f_str;
            f_str << arg_f;
            result.append(f_str.str());
          }
          ++i;
          break;
        }
        // %O and %o mean that call argument should be treated as object
        // and the specifier should be replaced with JS generic object
        // formatting of this object or optimally useful formatting.
        // This is non-trivial task. For example, Node.JS uses special
        // util.inspect() function to produce such formatting.
        //
        // For now we take simpler approach and use V8 Value::ToDetailString()
        // method which produces string representation useful for debugging
        // purposes. Unfortunately, for objects it doesn't show much info.
        //
        // TODO: Implement something similar to util.inspect() which will
        //       print object properties, methods and so on.
        case 'o':
        case 'O': {
          // ToDetailString() should succeed, unless execution is
          // terminated. Try to handle the latter case gracefully.
          v8::TryCatch try_catch(isolate);
          v8::Local<v8::String> arg_det_str;
          if (!(args[i]
                    ->ToDetailString(isolate->GetCurrentContext())
                    .ToLocal(&arg_det_str))) {
            args.GetReturnValue().Set(try_catch.ReThrow());
            return std::nullopt;
          }
          // Converting JS String to UTF-8 should not fail (unless OOM).
          v8::String::Utf8Value val(isolate, arg_det_str);
          result.append(*val, val.length());
          ++i;
          break;
        }
        // %c indicates that argument contains CSS which should be used to
        // style further text logged. In our case we simple omit this
        // specifier and call argument corresponding to it.
        case 'c': {
          ++i;
          break;
        }
        // %% is replaced with %. It doesn't consume call argument.
        case '%': {
          result.push_back('%');
          break;
        }
        // Not one of supported specifiers. Keep % and char that follows as is.
        default:
          result.push_back('%');
          result.push_back(*(pos + 1));
          break;
      }
      chunk_start = pos + 2;
    }
  }

  // In case when there is only one argument or the first argument is not
  // a String, or there are extra arguments left after processing format
  // specifiers on the previous stage, we convert these unprocessed
  // arguments to string representations useful for debugging purposes
  // and add to result concatenation of these strings separated by ' '.
  while (i < args.Length()) {
    // ToDetailString() should succeed, unless execution is
    // terminated. Try to handle the latter case gracefully.
    v8::TryCatch try_catch(isolate);
    v8::Local<v8::String> arg_det_str;
    if (!(args[i]
              ->ToDetailString(isolate->GetCurrentContext())
              .ToLocal(&arg_det_str))) {
      args.GetReturnValue().Set(try_catch.ReThrow());
      return std::nullopt;
    }
    // Converting JS String to UTF-8 should not fail (unless OOM).
    v8::String::Utf8Value val(isolate, arg_det_str);
    if (i > first_arg) result.push_back(' ');
    result.append(*val, val.length());
    ++i;
  }
  return result;
}

void Js_console::build_and_write_log(
    Log_level_type level, const char *prepend_message, int first_arg,
    const v8::FunctionCallbackInfo<v8::Value> &args) {
  // Per console standard we should log nothing if our log()-like call
  // didn't get any arguments.
  if (prepend_message != nullptr || (args.Length() > first_arg)) {
    auto opt_log_line = build_message(prepend_message, first_arg, args);

    if (!opt_log_line.has_value()) {
      // Error while building message, exception object has been stored
      // in return value already.
      return;
    }

    // Get Auth_id_context in which calling JavaScript code is run.
    //
    // TODO: Here in other similar places - consider if ther is a better
    //       way to get console object. Perhaps we can store pointer to
    //       Auth_id_ctx in global object as internal field?
    //       Or even pointer to Js_console object itself in our JS
    //       console object (i.e. "this" of the current JS call).
    auto auth_id_ctx = Js_thd::get_current_auth_id_context();
    auth_id_ctx->console.write_log(level, std::move(*opt_log_line));
  }

  args.GetReturnValue().Set(v8::Undefined(args.GetIsolate()));
}

void Js_console::prepare_object(v8::Local<v8::Context> context) {
  v8::Isolate *isolate = context->GetIsolate();

  v8::HandleScope handle_scope(isolate);

  // Replace built-in console object (which doesn't do anything by default) with
  // out own implementation.
  //
  // Due to presence of this built-in we cannot simply add template for our own
  // console to template for global object.
  //
  // TODO: Study pros and cons of using/implementing V8's non-public
  //       debug::ConsoleDelegate API instead, like D8 does. Perhaps we can
  //       get support for some format specifiers like %d and %f for free
  //       thanks to this?
  //
  // TODO: Study if console object methods should be made read-only and
  //       non-deletable.
  v8::Local<v8::ObjectTemplate> console_templ =
      v8::ObjectTemplate::New(isolate);

  /*
    Implementation of console.assert([condition], ...data) call.
    It is a conditional form of error(...data) call.
  */
  auto console_assert_cb_lambda =
      [](const v8::FunctionCallbackInfo<v8::Value> &args) -> void {
    v8::Isolate *isolate = args.GetIsolate();
    if (args.Length() == 0 || !args[0]->BooleanValue(isolate)) {
      if (args.Length() > 1) {
        build_and_write_log(Log_level_type::ERROR, "Assertion failed: ", 1,
                            args);
      } else {
        build_and_write_log(Log_level_type::ERROR, "Assertion failed", 1, args);
      }
    }
  };
  console_templ->Set(
      isolate, "assert",
      v8::FunctionTemplate::New(isolate, console_assert_cb_lambda));

  /*
    Implementations of console.debug(...data), error(...data), info(...data),
    log(...data) and warn(...data) calls. They only differ in log level used.

    Also, in our case log() call is just an alias for info() call.
  */
  auto console_debug_cb_lambda =
      [](const v8::FunctionCallbackInfo<v8::Value> &args) -> void {
    build_and_write_log(Log_level_type::DEBUG, nullptr, 0, args);
  };
  console_templ->Set(
      isolate, "debug",
      v8::FunctionTemplate::New(isolate, console_debug_cb_lambda));

  auto console_error_cb_lambda =
      [](const v8::FunctionCallbackInfo<v8::Value> &args) -> void {
    build_and_write_log(Log_level_type::ERROR, nullptr, 0, args);
  };
  console_templ->Set(
      isolate, "error",
      v8::FunctionTemplate::New(isolate, console_error_cb_lambda));

  auto console_info_cb_lambda =
      [](const v8::FunctionCallbackInfo<v8::Value> &args) -> void {
    build_and_write_log(Log_level_type::INFO, nullptr, 0, args);
  };
  console_templ->Set(
      isolate, "info",
      v8::FunctionTemplate::New(isolate, console_info_cb_lambda));
  console_templ->Set(
      isolate, "log",
      v8::FunctionTemplate::New(isolate, console_info_cb_lambda));

  auto console_warn_cb_lambda =
      [](const v8::FunctionCallbackInfo<v8::Value> &args) -> void {
    build_and_write_log(Log_level_type::WARNING, nullptr, 0, args);
  };
  console_templ->Set(
      isolate, "warn",
      v8::FunctionTemplate::New(isolate, console_warn_cb_lambda));

  /*
    Implementation of console.count([label]) call.
  */
  auto console_count_cb_lambda =
      [](const v8::FunctionCallbackInfo<v8::Value> &args) -> void {
    v8::Isolate *isolate = args.GetIsolate();

    // Console standard says to use 'default' if no label provided.
    std::string label("default");

    /*
      We follow standard JS practice and allow passing to this function
      more arguments than one argument it should take per specification.
      We just ignore those extra arguments.

      This allows to directly use this call as a callback to, e.g.
      Array.prototype.forEach() method.
    */
    if (args.Length() > 0) {
      // Conversion to String might fail. Handle that gracefully
      // by ensuring that exception generated is passed to caller.
      v8::TryCatch try_catch(isolate);
      v8::Local<v8::String> arg_str;
      if (!(args[0]
                ->ToString(isolate->GetCurrentContext())
                .ToLocal(&arg_str))) {
        args.GetReturnValue().Set(try_catch.ReThrow());
        return;
      }

      v8::String::Utf8Value utf8(isolate, arg_str);
      // Converting JS String to UTF-8 should not fail (unless OOM).
      label = std::string(*utf8, utf8.length());
    }

    auto auth_id_ctx = Js_thd::get_current_auth_id_context();
    auto count = auth_id_ctx->console.count(label);

    std::string log_line;
    log_line.append(label);
    log_line.append(": ");
    log_line.append(std::to_string(count));

    auth_id_ctx->console.write_log(Log_level_type::INFO, std::move(log_line));

    args.GetReturnValue().Set(v8::Undefined(isolate));
  };
  console_templ->Set(
      isolate, "count",
      v8::FunctionTemplate::New(isolate, console_count_cb_lambda));

  /*
    Implementation of console.countReset([label]) call.
  */
  auto console_count_reset_cb_lambda =
      [](const v8::FunctionCallbackInfo<v8::Value> &args) -> void {
    v8::Isolate *isolate = args.GetIsolate();

    // Console standard says to use 'default' if no label provided.
    std::string label("default");

    // Again we follow standard JS practice and just ignore extra arguments.
    if (args.Length() > 0) {
      // Conversion to String might fail. Handle that gracefully
      // by ensuring that exception generated is passed to caller.
      v8::TryCatch try_catch(isolate);
      v8::Local<v8::String> arg_str;
      if (!(args[0]
                ->ToString(isolate->GetCurrentContext())
                .ToLocal(&arg_str))) {
        args.GetReturnValue().Set(try_catch.ReThrow());
        return;
      }

      v8::String::Utf8Value utf8(isolate, arg_str);
      // Converting JS String to UTF-8 should not fail (unless OOM).
      label = std::string(*utf8, utf8.length());
    }

    auto auth_id_ctx = Js_thd::get_current_auth_id_context();

    if (auth_id_ctx->console.count_reset(label)) {
      std::string log_line;
      log_line.append(label);
      log_line.append(": there is no counter to reset");
      auth_id_ctx->console.write_log(Log_level_type::WARNING,
                                     std::move(log_line));
    }

    args.GetReturnValue().Set(v8::Undefined(isolate));
  };
  console_templ->Set(
      isolate, "countReset",
      v8::FunctionTemplate::New(isolate, console_count_reset_cb_lambda));

  /*
    Implementation of console.group([...label]) and groupCollapsed([...label])
    calls. The latter is alias for the former in our case.
  */
  auto console_group_cb_lambda =
      [](const v8::FunctionCallbackInfo<v8::Value> &args) -> void {
    if (args.Length() > 0) {
      // Build group label using the same logic as we produce
      // message in log() call.
      auto opt_group_label = build_message(nullptr, 0, args);

      // In case of error no value is returned, and the exception
      // object already stored as a return value of the call.
      if (opt_group_label.has_value()) {
        auto auth_id_ctx = Js_thd::get_current_auth_id_context();
        auth_id_ctx->console.start_group(*opt_group_label);
      }
    } else {
      // If no argument is provided we start group with special
      // null label. The label/group start is not logged to console
      // in this case.
      auto auth_id_ctx = Js_thd::get_current_auth_id_context();
      auth_id_ctx->console.start_group(std::nullopt);
    }

    args.GetReturnValue().Set(v8::Undefined(args.GetIsolate()));
  };
  console_templ->Set(
      isolate, "group",
      v8::FunctionTemplate::New(isolate, console_group_cb_lambda));
  console_templ->Set(
      isolate, "groupCollapsed",
      v8::FunctionTemplate::New(isolate, console_group_cb_lambda));

  /*
    Implementation of console.groupEnd() function.
  */
  auto console_group_end_cb_lambda =
      [](const v8::FunctionCallbackInfo<v8::Value> &args) -> void {
    // We follow standard JS practice and just ignore extra (i.e. all)
    // arguments.
    auto auth_id_ctx = Js_thd::get_current_auth_id_context();
    auth_id_ctx->console.end_group();
    args.GetReturnValue().Set(v8::Undefined(args.GetIsolate()));
  };
  console_templ->Set(
      isolate, "groupEnd",
      v8::FunctionTemplate::New(isolate, console_group_end_cb_lambda));

  /*
    Implementation of console.time([label]) call.
  */
  auto console_time_cb_lambda =
      [](const v8::FunctionCallbackInfo<v8::Value> &args) -> void {
    v8::Isolate *isolate = args.GetIsolate();

    /*
      Per standard absent label means 'default' should be used.
      And again follow standard JS practice and ignore extra arguments.
    */
    std::string label("default");

    if (args.Length() > 0) {
      // Conversion to String might fail. Handle that gracefully
      // by ensuring that exception generated is passed to caller.
      v8::TryCatch try_catch(isolate);
      v8::Local<v8::String> arg_str;
      if (!(args[0]
                ->ToString(isolate->GetCurrentContext())
                .ToLocal(&arg_str))) {
        args.GetReturnValue().Set(try_catch.ReThrow());
        return;
      }

      v8::String::Utf8Value utf8(isolate, arg_str);
      // Converting JS String to UTF-8 should not fail (unless OOM).
      label = std::string(*utf8, utf8.length());
    }

    auto auth_id_ctx = Js_thd::get_current_auth_id_context();

    if (auth_id_ctx->console.start_timer(label)) {
      std::string log_line;
      log_line.append(label);
      log_line.append(": timer has been started already");
      auth_id_ctx->console.write_log(Log_level_type::WARNING,
                                     std::move(log_line));
    }

    args.GetReturnValue().Set(v8::Undefined(isolate));
  };
  console_templ->Set(
      isolate, "time",
      v8::FunctionTemplate::New(isolate, console_time_cb_lambda));

  /*
    Implementation of console.timeLog([label] [...data]) call.
  */
  auto console_time_log_cb_lambda =
      [](const v8::FunctionCallbackInfo<v8::Value> &args) -> void {
    v8::Isolate *isolate = args.GetIsolate();

    // Similarly to time() we use 'default' label if one is not provided.
    std::string label("default");

    if (args.Length() > 0) {
      // Conversion to String might fail. Handle that gracefully
      // by ensuring that exception generated is passed to caller.
      v8::TryCatch try_catch(isolate);
      v8::Local<v8::String> arg_str;
      if (!(args[0]
                ->ToString(isolate->GetCurrentContext())
                .ToLocal(&arg_str))) {
        args.GetReturnValue().Set(try_catch.ReThrow());
        return;
      }

      v8::String::Utf8Value utf8(isolate, arg_str);
      // Converting JS String to UTF-8 should not fail (unless OOM).
      label = std::string(*utf8, utf8.length());
    }

    std::stringstream log_line_str;
    log_line_str << label << ": ";

    auto auth_id_ctx = Js_thd::get_current_auth_id_context();

    auto opt_elapsed = auth_id_ctx->console.get_timer(label);

    if (opt_elapsed.has_value()) {
      // We output elapsed time in milliseconds, but with microsecond
      // precision (this is similar to Node.JS).
      double elapsed =
          std::chrono::duration_cast<std::chrono::microseconds>(*opt_elapsed)
              .count() /
          1000.0;

      log_line_str << std::fixed << std::setprecision(3) << elapsed << " ms";

      // We convert values of 2nd and later arguments to string form useful
      // for debugging purposes and add to result concatenation of these
      // strings separated by ' '.
      int i = 1;
      while (i < args.Length()) {
        // Gracefully handle the case when ToDetailString() fails
        // due to execution termination.
        v8::TryCatch try_catch(isolate);
        v8::Local<v8::String> arg_det_str;
        if (!(args[i]
                  ->ToDetailString(isolate->GetCurrentContext())
                  .ToLocal(&arg_det_str))) {
          args.GetReturnValue().Set(try_catch.ReThrow());
          return;
        }
        // Converting JS String to UTF-8 should not fail (unless OOM).
        v8::String::Utf8Value val(isolate, arg_det_str);

        log_line_str << ' ' << std::string_view(*val, val.length());
        ++i;
      }
      auth_id_ctx->console.write_log(Log_level_type::INFO,
                                     std::move(log_line_str.str()));
    } else {
      log_line_str << "no such timer";
      auth_id_ctx->console.write_log(Log_level_type::WARNING,
                                     std::move(log_line_str.str()));
    }

    args.GetReturnValue().Set(v8::Undefined(isolate));
  };
  console_templ->Set(
      isolate, "timeLog",
      v8::FunctionTemplate::New(isolate, console_time_log_cb_lambda));

  /*
    Implementation of console.timeEnd([label]) call.
  */
  auto console_time_end_cb_lambda =
      [](const v8::FunctionCallbackInfo<v8::Value> &args) -> void {
    v8::Isolate *isolate = args.GetIsolate();

    /*
      Similarly to time() we use 'default' label if one is not provided
      and ignore extra arguments if there are more than one.
    */
    std::string label("default");

    if (args.Length() > 0) {
      // Pass exception to the caller if conversion to String fails.
      v8::TryCatch try_catch(isolate);
      v8::Local<v8::String> arg_str;
      if (!(args[0]
                ->ToString(isolate->GetCurrentContext())
                .ToLocal(&arg_str))) {
        args.GetReturnValue().Set(try_catch.ReThrow());
        return;
      }

      // Converting JS String to UTF-8 should not fail (unless OOM).
      v8::String::Utf8Value utf8(isolate, arg_str);
      label = std::string(*utf8, utf8.length());
    }

    std::stringstream log_line_str;
    log_line_str << label << ": ";

    auto auth_id_ctx = Js_thd::get_current_auth_id_context();

    auto opt_elapsed = auth_id_ctx->console.end_timer(label);

    if (opt_elapsed.has_value()) {
      // We output elapsed time in milliseconds, but with microsecond
      // precision (this is similar to Node.JS).
      double elapsed =
          std::chrono::duration_cast<std::chrono::microseconds>(*opt_elapsed)
              .count() /
          1000.0;

      log_line_str << std::fixed << std::setprecision(3) << elapsed << " ms";

      auth_id_ctx->console.write_log(Log_level_type::INFO,
                                     std::move(log_line_str.str()));
    } else {
      log_line_str << "no such timer";
      auth_id_ctx->console.write_log(Log_level_type::WARNING,
                                     std::move(log_line_str.str()));
    }

    args.GetReturnValue().Set(v8::Undefined(isolate));
  };
  console_templ->Set(
      isolate, "timeEnd",
      v8::FunctionTemplate::New(isolate, console_time_end_cb_lambda));

  /*
    Implementation of console.clear() call.
  */
  auto console_clear_cb_lambda =
      [](const v8::FunctionCallbackInfo<v8::Value> &args) -> void {
    v8::Isolate *isolate = args.GetIsolate();

    // Similarly to other calls we follow standard JS practice and ignore
    // extra arguments, which means all arguments in this case.

    auto auth_id_ctx = Js_thd::get_current_auth_id_context();

    auth_id_ctx->console.clear_log();
    auth_id_ctx->console.reset_group_stack();

    args.GetReturnValue().Set(v8::Undefined(isolate));
  };
  console_templ->Set(
      isolate, "clear",
      v8::FunctionTemplate::New(isolate, console_clear_cb_lambda));

  /*
    console.table(), trace(), dir() and dirxml() are not implement yet.
    So we install no-op handler for them.
  */
  auto console_noop_cb_lambda =
      [](const v8::FunctionCallbackInfo<v8::Value> &args) -> void {
    args.GetReturnValue().Set(v8::Undefined(args.GetIsolate()));
  };
  console_templ->Set(
      isolate, "table",
      v8::FunctionTemplate::New(isolate, console_noop_cb_lambda));
  console_templ->Set(
      isolate, "trace",
      v8::FunctionTemplate::New(isolate, console_noop_cb_lambda));
  console_templ->Set(
      isolate, "dir",
      v8::FunctionTemplate::New(isolate, console_noop_cb_lambda));
  console_templ->Set(
      isolate, "dirxml",
      v8::FunctionTemplate::New(isolate, console_noop_cb_lambda));

  // Failure to create 'console' object instance probably means
  // OOM or logic error in our case, so we don't try to handle it
  // gracefully.
  v8::Local<v8::Object> console =
      console_templ->NewInstance(context).ToLocalChecked();

  context->Global()
      ->Set(context, v8::String::NewFromUtf8Literal(isolate, "console"),
            console)
      .Check();  // Set() is not supposed to fail per V8 docs.
}

std::string Js_console::get_log() const {
  // TODO: See what users will say about the fact that we print messages with
  //       Debug level by default (browsers don't do this). Does it create
  //       too much noise? Perhaps separate less noisy version of UDF
  //       should be implemented or alternatively we should not print
  //       Debug messages by default, but make them available through
  //       separate UDF with _VERBOSE suffix instead.
  std::string result;

  for (const auto &line : m_log) {
    constexpr size_t GROUP_INDENT_SIZE = 2;
    // TODO: Consider indenting all lines from the message, and not only
    //       the part before first \n (Node.JS does this).
    //       Requires splitting message on \n before output.
    result.append(GROUP_INDENT_SIZE * line.group_level, ' ');
    result.append(line.message);
    result.append("\n");
  }

  return result;
}

/**
  Helper which adds string representation of log entry timestamp into JSON.
*/
static void put_ts(Json_writer *json_writer, Js_console::log_line_ts_type ts) {
  // Prepare broken-down rerpresentation timestamp in SYSTEM time zone.
  time_t t = Js_console::log_line_clock::to_time_t(ts);
  struct tm t_tm;
  localtime_r(&t, &t_tm);

  // Calculate fractional part of timestamp in microseconds.
  auto micros = std::chrono::duration_cast<std::chrono::microseconds>(
                    ts - floor<std::chrono::seconds>(ts))
                    .count();

  char ts_buff[sizeof("YYYY-MM-DD HH:MM:SS.FFFFFF")];
  size_t dot_pos = strftime(ts_buff, sizeof(ts_buff), "%F %T.", &t_tm);
  snprintf(ts_buff + dot_pos, sizeof(ts_buff) - dot_pos, "%06u",
           static_cast<unsigned int>(micros));

  json_writer->String(ts_buff);
}

/**
  Helper which adds string representation of console log level into JSON.
*/
static void put_log_level(Json_writer *json_writer,
                          Js_console::Log_level_type level) {
  switch (level) {
    case Js_console::Log_level_type::ERROR:
      json_writer->String(STRING_WITH_LEN("Error"));
      break;
    case Js_console::Log_level_type::WARNING:
      json_writer->String(STRING_WITH_LEN("Warning"));
      break;
    case Js_console::Log_level_type::INFO:
      json_writer->String(STRING_WITH_LEN("Info"));
      break;
    case Js_console::Log_level_type::DEBUG:
      json_writer->String(STRING_WITH_LEN("Debug"));
      break;
    default:
      assert(false);
      json_writer->String(STRING_WITH_LEN(""));  // Safety.
      break;
  }
}

std::string Js_console::get_group_stack_json() const {
  // Use fast-path for the most common case.
  if (m_group_stack.empty()) return std::string("[]");

  Json_string_buffer string_buffer;
  Json_writer json_writer(string_buffer);
  // We want to use more compact representation for groups array.
  json_writer.SetFormatOptions(rapidjson::kFormatSingleLineArray);

  json_writer.StartArray();
  for (const auto &group : m_group_stack) {
    if (group.has_value()) {
      json_writer.String(*group);
    } else {
      json_writer.Null();
    }
  }
  json_writer.EndArray();

  return std::string(string_buffer.GetString(), string_buffer.GetSize());
}

std::string Js_console::get_log_json() const {
  Json_string_buffer string_buffer;
  Json_writer json_writer(string_buffer);

  json_writer.StartArray();

  for (const auto &line : m_log) {
    json_writer.StartObject();

    json_writer.Key(STRING_WITH_LEN("timestamp"));
    put_ts(&json_writer, line.ts);

    json_writer.Key(STRING_WITH_LEN("level"));
    put_log_level(&json_writer, line.level);

    if (line.group_level != 0) {
      json_writer.Key(STRING_WITH_LEN("groups"));
      json_writer.RawValue(line.group_stack_json.c_str(),
                           line.group_stack_json.length(),
                           rapidjson::kArrayType);
    }

    if (line.is_group_label) {
      json_writer.Key(STRING_WITH_LEN("isGroupLabel"));
      json_writer.Bool(true);
    }

    json_writer.Key(STRING_WITH_LEN("message"));
    json_writer.String(line.message);

    json_writer.EndObject();
  }

  json_writer.EndArray();

  return std::string(string_buffer.GetString(), string_buffer.GetSize());
}

constexpr unsigned int MAX_LOG_SIZE_DEFAULT = 1024 * 1024;
unsigned int Js_console::s_max_log_size = MAX_LOG_SIZE_DEFAULT;

bool Js_console::register_sys_var() {
  INTEGRAL_CHECK_ARG(uint) max_console_log_size_check;

  // We use 1Mb as a default for max console buffer size which translates
  // to approximately 10000 of lines. It should be enough for most of uses
  // (e.g. it is a common default for terminal backscroll buffer on Linux).
  max_console_log_size_check.def_val = MAX_LOG_SIZE_DEFAULT;
  // Console Standard suggests that we should be able to buffer at least
  // on the order of 100 lines, which is close enough to 10K,
  max_console_log_size_check.min_val = 10 * 1024;
  // 1Gb of console buffer should be enough for everyone!
  max_console_log_size_check.max_val = 1024 * 1024 * 1024;
  max_console_log_size_check.blk_sz = 0;

  if (mysql_service_component_sys_variable_register->register_variable(
          CURRENT_COMPONENT_NAME_STR, MAX_CONSOLE_LOG_SIZE_VAR_NAME,
          PLUGIN_VAR_INT | PLUGIN_VAR_UNSIGNED,
          "Approximate maximum size of messages in bytes which are allowed "
          "to be buffered by JS console for each connection/user pair.",
          nullptr, nullptr, (void *)&max_console_log_size_check,
          (void *)&Js_console::s_max_log_size)) {
    // Implementation of component_sys_variable_register service is non-trivial,
    // and can fail for reasons other than OOM, so we play safe and try to
    // handle error gracefully here.
    my_error(ER_LANGUAGE_COMPONENT, MYF(0),
             "Can't register " MAX_CONSOLE_LOG_SIZE_VAR_NAME
             " system variable for " CURRENT_COMPONENT_NAME_STR " component.");
    return true;
  }

  return false;
}

bool Js_console::unregister_sys_var() {
  if (mysql_service_component_sys_variable_unregister->unregister_variable(
          CURRENT_COMPONENT_NAME_STR, "max_console_log_size")) {
    // The above should not fail normally. Still we play safe.
    my_error(ER_LANGUAGE_COMPONENT, MYF(0),
             "Can't unregister " MAX_CONSOLE_LOG_SIZE_VAR_NAME
             " system variable for " CURRENT_COMPONENT_NAME_STR " component.");
    return true;
  }
  return false;
}
