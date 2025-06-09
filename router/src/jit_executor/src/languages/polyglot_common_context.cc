/*
 * Copyright (c) 2024, 2025, Oracle and/or its affiliates.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2.0,
 * as published by the Free Software Foundation.
 *
 * This program is designed to work with certain software (including
 * but not limited to OpenSSL) that is licensed under separate terms,
 * as designated in a particular file or component or in included license
 * documentation.  The authors of MySQL hereby grant you an additional
 * permission to link the program and your derivative works with the
 * separately licensed software that they have either included with
 * the program or referenced in the documentation.
 *
 * This program is distributed in the hope that it will be useful,  but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 * the GNU General Public License, version 2.0, for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "languages/polyglot_common_context.h"

#include <cstring>

#include "mysql/harness/scoped_callback.h"
#include "native_wrappers/polyglot_collectable.h"
#include "utils/polyglot_scope.h"
#include "utils/polyglot_utils.h"

namespace shcore {
namespace polyglot {

namespace {
// Function to convert a vector of strings to an array of character pointers
char **get_char_ptr(const std::vector<std::string> &vec) {
  if (vec.empty()) {
    return nullptr;
  }

  // Allocate memory for the array of character pointers
  char **arr = new char *[vec.size() + 1];
  arr[0] = nullptr;

  // Copy each string from the vector into the array
  for (size_t i = 0; i < vec.size(); ++i) {
    arr[i + 1] = new char[vec[i].length() + 1];
    strcpy(arr[i + 1], vec[i].c_str());
  }

  return arr;
}
}  // namespace

void Polyglot_common_context::initialize(
    const std::vector<std::string> &isolate_args) {
  if (!isolate_args.empty()) {
    char **params = get_char_ptr(isolate_args);
    mysql_harness::ScopedCallback release([&]() { delete[] params; });

    poly_isolate_params isolate_params;
    if (poly_ok != poly_set_isolate_params(&isolate_params,
                                           isolate_args.size() + 1, params)) {
      throw Polyglot_generic_error("Error creating polyglot isolate params");
    }

    if (poly_ok !=
        poly_create_isolate(&isolate_params, &m_isolate, &m_thread)) {
      throw Polyglot_generic_error("Error creating polyglot isolate");
    }
  } else {
    if (poly_ok != poly_create_isolate(NULL, &m_isolate, &m_thread)) {
      throw Polyglot_generic_error("Error creating polyglot isolate");
    }
  }

  m_scope = std::make_unique<Polyglot_scope>(m_thread);

  if (const auto rc = poly_register_log_handler_callbacks(
          m_thread, &log_callback, &flush_callback, &fatal_error_callback,
          static_cast<void *>(this));
      rc != poly_ok) {
    throw Polyglot_error(m_thread, rc);
  }

  // Initialize the engine to be used in the context builder, not defining a
  // custom engine (i.e. getting nullptr as result) enforces the default logic
  // on GraalVM which is triggers the creation of a custom engine per created
  // context.
  init_engine();
}

void Polyglot_common_context::finalize() {
  m_engine.reset();

  m_scope.reset();

  if (m_isolate && m_thread) {
    if (const auto rc = poly_detach_all_threads_and_tear_down_isolate(m_thread);
        rc != poly_ok) {
      std::string error{"polyglot error while tearing down the isolate"};
      log(error.data(), error.size());
    }
  }

  clean_collectables();
}

void Polyglot_common_context::fatal_error_callback(void *data) {
  auto self = static_cast<Polyglot_common_context *>(data);
  self->fatal_error();
}

void Polyglot_common_context::flush_callback(void *data) {
  auto self = static_cast<Polyglot_common_context *>(data);
  self->flush();
}

void Polyglot_common_context::log_callback(const char *bytes, size_t length,
                                           void *data) {
  auto self = static_cast<Polyglot_common_context *>(data);
  self->log(bytes, length);
}

void Polyglot_common_context::init_engine() {
  auto engine = create_engine();
  if (engine) {
    m_engine = Store(m_thread, engine);
  }
}

void Polyglot_common_context::clean_collectables() { m_registry.clean(); }

}  // namespace polyglot
}  // namespace shcore