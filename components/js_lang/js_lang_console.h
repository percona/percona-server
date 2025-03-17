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
  This and corresponding .cc file contain implementation of debugging console
  support for our JS routines.

  See also comment for js_lang_console.cc file for high-level design
  considerations for this debugging console implementation.
*/

#ifndef COMPONENT_JS_LANG_JS_LANG_CONSOLE_H
#define COMPONENT_JS_LANG_JS_LANG_CONSOLE_H

#include <chrono>
#include <deque>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <v8.h>

/**
  Class which represents JS debugging console instance for specific
  user and connection pair.
*/
class Js_console {
 public:
  /**
    Enum type for severity level of messages stored in console log.

    @note This is really an implementation detail, it is public only to
          simplify implementation of JSON printer for this type.
  */
  enum class Log_level_type { ERROR, WARNING, INFO, DEBUG };

  /**
    Clock and timestamp types for messages stored in console log.

    These are really implementation details, they are public only to
    simplify implementation of JSON printer for timestamps.
  */
  using log_line_clock = std::chrono::system_clock;
  using log_line_ts_type = log_line_clock::time_point;

  // Use default implementation of ctor.
  Js_console() = default;

  // Block default copy/move semantics.
  Js_console(Js_console const &rhs) = delete;
  Js_console(Js_console &&rhs) = delete;
  Js_console &operator=(Js_console const &rhs) = delete;
  Js_console &operator=(Js_console &&rhs) = delete;

  /** Prepare 'console' object and its methods for the JS context. */
  static void prepare_object(v8::Local<v8::Context> context);

  /** Get simplified version of console log contents. */
  std::string get_log() const;

  /** Get full version of console log contents in JSON format. */
  std::string get_log_json() const;

  /** Clear console log. Returns number of messages which were removed. */
  size_t clear_log() {
    size_t result = m_log.size();
    m_approx_log_size = 0;
    m_log.clear();
    m_log.shrink_to_fit();
    return result;
  }

  /**
    Register/unregister system variable which controls JS console
    maximum log size.

    @retval False - Success.
    @retval True  - Failure (error has been reported).
  */
  static bool register_sys_var();
  static bool unregister_sys_var();

 private:
  /**
    Helper struct which represents console log entry/bundles all the
    information which we store for each message logged to console.
  */
  struct Log_line {
    log_line_ts_type ts;
    Log_level_type level;
    size_t group_level;
    std::string group_stack_json;
    bool is_group_label;
    std::string message;

    Log_line(log_line_ts_type ts_arg, Log_level_type level_arg,
             size_t group_level_arg, std::string &&group_stack_json_arg,
             bool group_label_arg, std::string &&message_arg)
        : ts(ts_arg),
          level(level_arg),
          group_level(group_level_arg),
          group_stack_json(std::move(group_stack_json_arg)),
          is_group_label(group_label_arg),
          message(std::move(message_arg)) {}

    // Use default implementation of move constructor.
    Log_line(Log_line &&rhs) = default;
    Log_line &operator=(Log_line &&rhs) = delete;

    // Block default copy semantics.
    Log_line(Log_line const &rhs) = delete;
    Log_line &operator=(Log_line const &rhs) = delete;

    /**
      Get approximate size of console log entry for the purpose of
      limiting log buffering.
    */
    size_t get_approx_size() const {
      return group_stack_json.length() + message.length();
    }
  };

  /**
    Clock which is used by time()/timeLog()/timeEnd() series of JS calls
    to measure time elapsed between them.
  */
  using timer_clock = std::chrono::steady_clock;

  /** Write string message with specified severity level to console log. */
  void write_log(Log_level_type level, std::string &&msg,
                 bool is_group_label = false);

  /**
    Build message from the arguments of one of console.log() family of JS
    calls and write it to the console log.

    @param level            Log level to use.
    @param prepend_message  String prepend to the built message before
                            writing (or nullptr).
    @param first_arg        Index of the first argument of JS call to
                            be processed.
    @param args             Object representing arguments and return value
                            of JS call.

    @note This helper implements core of console.log(), info(), error() and
          some other similar JS calls.
  */
  static void build_and_write_log(
      Log_level_type level, const char *prepend_message, int first_arg,
      const v8::FunctionCallbackInfo<v8::Value> &args);

  /**
    Increment counter in console's counters table for the label.

    @return Updated counter value.
  */
  size_t count(const std::string &label) { return ++(m_counters[label]); }

  /**
    'Reset' counter in console's counters table for the label.

    @retval False on success.
    @retval True if no counter were present in the table for the label.

    @note Console standard says that countReset() call should simply
          reset counter table for label to 0. However, implementations,
          like browsers or Node.JS, delete entry from the counters table
          instead. We stick to the latter approach and delete entry
          instead of zeroing it out as well.
  */
  bool count_reset(const std::string &label) {
    auto it = m_counters.find(label);
    if (it == m_counters.end()) return true;
    m_counters.erase(it);
    return false;
  }

  /**
    Push new group with the label provided to the top of console group stack,
    if label is present add log message with group label to the console.
  */
  void start_group(const std::optional<std::string> &&label) {
    if (label.has_value()) {
      std::string label_msg(*label);
      write_log(Log_level_type::INFO, std::move(label_msg), true);
    }
    m_group_stack.push_back(std::move(label));
  }

  /** Pop group from the top of console group stack, if present. */
  void end_group() {
    if (!m_group_stack.empty()) m_group_stack.pop_back();
  }

  /** Reset console group stack. */
  void reset_group_stack() { m_group_stack.clear(); }

  /** Get JSON array representing current state of console group stack. */
  std::string get_group_stack_json() const;

  /**
    Start timer and add it to console timers table under the label.

    @retval False - success.
    @retval True - there is already timer for this label in the table.
  */
  bool start_timer(const std::string &label) {
    auto it = m_timers.find(label);
    if (it != m_timers.end()) return true;
    m_timers.emplace(label, timer_clock::now());
    return false;
  }

  /**
    Get time elapsed for the timer with the label.

    @return Elapsed time or std::nullopt if there is no timer active for
            the label.
  */
  std::optional<timer_clock::duration> get_timer(const std::string &label) {
    auto it = m_timers.find(label);
    if (it == m_timers.end()) return std::nullopt;
    return (timer_clock::now() - it->second);
  }

  /**
    Stop timer for the label (and delete it from timers table).

    @return Elapsed time or std::nullopt if there is no timer active for
            the label.
  */
  std::optional<timer_clock::duration> end_timer(const std::string &label) {
    auto it = m_timers.find(label);
    if (it == m_timers.end()) return std::nullopt;
    auto result = timer_clock::now() - it->second;
    m_timers.erase(it);
    return result;
  }

  /**
    Buffered console log lines.

    @note New lines are added to the back of the buffer. Old lines are
          removed from its front once limit on size of buffered log lines
          is exceeded.
  */
  std::deque<Log_line> m_log;

  /**
    Approximate size of log lines which are buffered (in bytes).

    @note We track approximate size, instead of exact one, since it is hard
          to define what actually exact size means. The exact size of memory
          representation doesn't make much sense to users, and size of
          user-visible representation may vary. So we go with good enough
          approximate size instead.
  */
  size_t m_approx_log_size = 0;

  /**
    Console's counters table (aka counter map).

    Used by JS console.count() and countReset() calls.
  */
  std::unordered_map<std::string, size_t> m_counters;

  /**
    Console's timers table.

    Used by JS console.time(), timeLog() and timeEnd() calls.
  */
  std::unordered_map<std::string, timer_clock::time_point> m_timers;

  /**
    Current group stack.

    Controlled by JS console.group(), groupCollapsed() and groupEnd() calls.
  */
  std::vector<std::optional<std::string>> m_group_stack;

  /**
    Approximate maximum size of console log lines which can be buffered
    (in bytes).
  */
  static unsigned int s_max_log_size;
};

#endif /* COMPONENT_JS_LANG_JS_LANG_CONSOLE_H */
