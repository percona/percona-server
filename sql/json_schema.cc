/* Copyright (c) 2018, 2026, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is designed to work with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have either included with
   the program or referenced in the documentation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "sql/json_schema.h"

#include <my_rapidjson_size_t.h>  // IWYU pragma: keep

#include <assert.h>
#include <rapidjson/document.h>
#include <rapidjson/error/error.h>
#include <rapidjson/memorystream.h>
#include <rapidjson/pointer.h>
#include <rapidjson/reader.h>
#include <rapidjson/schema.h>
#include <rapidjson/stringbuffer.h>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <string>
#include <unordered_set>
#include <utility>

#include "my_alloc.h"

#include "my_inttypes.h"
#include "my_sys.h"
#include "mysqld_error.h"
#include "sql-common/json_syntax_check.h"
#include "sql/sql_exception_handler.h"

/**
  Maximum "expansion depth" of a JSON Schema during rapidjson schema
  compilation.

  rapidjson schema compilation is recursive. The call stack depth is driven by
  a combination of:
  - JSON nesting within the schema (object members / array elements), and
  - local `$ref` expansion (compiling referenced subschemas recursively).

  This cap bounds the maximum number of edges along any path in the schema's
  "expansion graph", where both JSON-tree edges and local `$ref` edges count
  as 1.

  Since MySQL rejects JSON documents deeper than 100 levels (see
  `sql-common/json_syntax_check.cc`), schemas requiring more than 100 levels of
  expansion cannot describe JSON documents supported by MySQL. We therefore use
  100 as the single guard limit here as well.
*/
static constexpr uint32_t JSON_SCHEMA_MAX_EXPANSION_DEPTH = 100;

/**
  Check whether a `$ref` value is a *local* JSON Pointer reference.

  We only guard references of the form `"#"` or `"#/..."`.

  @param ref_value The value of the `$ref` member.
  @retval true The `$ref` is a local JSON pointer reference.
  @retval false Otherwise.
*/
static bool is_local_json_pointer_ref(const rapidjson::Value &ref_value) {
  if (!ref_value.IsString()) return false;
  const char *s = ref_value.GetString();
  const size_t len = ref_value.GetStringLength();
  return len > 0 && s[0] == '#' && (len == 1 || s[1] == '/');
}

/**
  Resolve a local JSON Pointer `$ref` against the schema document.

  @param schema_document The full JSON schema document.
  @param ref_value The `$ref` member value (must be a local JSON pointer).
  @returns Pointer to the referenced schema node on success, otherwise nullptr.
*/
static const rapidjson::Value *resolve_local_json_pointer_ref(
    const rapidjson::Document &schema_document,
    const rapidjson::Value &ref_value) {
  assert(is_local_json_pointer_ref(ref_value));
  const rapidjson::Pointer pointer(ref_value.GetString(),
                                   ref_value.GetStringLength());
  if (!pointer.IsValid()) return nullptr;
  return pointer.Get(schema_document);
}

/**
  Check whether any path from a schema node exceeds the expansion depth limit.

  The expansion depth is the number of edges along a path in the schema's
  expansion graph:
  - JSON-tree edges (object members / array elements), and
  - local `$ref` edges (local JSON Pointer references).

  `visiting` is used to break `$ref` cycles (cycles are treated as
  non-contributing; rapidjson will report cyclic references separately).

  @param schema_document Parsed JSON schema document (used for resolving
  `$ref`).
  @param node The node to start the search from.
  @param depth_so_far Current depth from the schema root.
  @param limit Maximum allowed expansion depth.
  @param[in,out] visiting Visitation set for cycle detection.
  @retval true The limit is exceeded along some path.
  @retval false No path exceeds the limit.
*/
static bool json_schema_exceeds_expansion_depth(
    const rapidjson::Document &schema_document, const rapidjson::Value *node,
    uint32_t depth_so_far, uint32_t limit,
    std::unordered_set<const rapidjson::Value *> *visiting) {
  assert(node != nullptr);
  assert(visiting != nullptr);

  if (depth_so_far > limit) {
    return true;
  }

  if (visiting->find(node) != visiting->end()) {
    // Cycle detected. rapidjson will report cyclic references separately; treat
    // this edge as non-contributing here to avoid infinite recursion.
    return false;
  }

  visiting->insert(node);

  // Local `$ref` edge.
  if (node->IsObject()) {
    const auto ref_itr = node->FindMember("$ref");
    if (ref_itr != node->MemberEnd() &&
        is_local_json_pointer_ref(ref_itr->value)) {
      const rapidjson::Value *target =
          resolve_local_json_pointer_ref(schema_document, ref_itr->value);
      if (target != nullptr &&
          json_schema_exceeds_expansion_depth(
              schema_document, target, depth_so_far + 1, limit, visiting)) {
        visiting->erase(node);
        return true;
      }
    }
  }

  // JSON-tree edges.
  if (node->IsObject()) {
    for (auto it = node->MemberBegin(); it != node->MemberEnd(); ++it) {
      const rapidjson::Value *child = &it->value;
      if (!child->IsObject() && !child->IsArray()) continue;
      if (json_schema_exceeds_expansion_depth(
              schema_document, child, depth_so_far + 1, limit, visiting)) {
        visiting->erase(node);
        return true;
      }
    }
  } else if (node->IsArray()) {
    for (auto it = node->Begin(); it != node->End(); ++it) {
      const rapidjson::Value *child = it;
      if (!child->IsObject() && !child->IsArray()) continue;
      if (json_schema_exceeds_expansion_depth(
              schema_document, child, depth_so_far + 1, limit, visiting)) {
        visiting->erase(node);
        return true;
      }
    }
  }

  visiting->erase(node);
  return false;
}

/**
  Validate that a JSON Schema does not require excessive recursion depth during
  rapidjson schema compilation.

  rapidjson schema compilation expands local `$ref` by recursively compiling the
  referenced subschemas. The recursion depth is therefore related to the number
  of local reference "hops" along the deepest expansion path. This depth can be
  large even if no referred object has a `$ref` at its own top level, as long as
  each referred object contains a nested object with a `$ref`.

  This function only checks whether any expansion path exceeds the limit. Each
  step into a nested object/array and each local `$ref` expansion counts as 1.

  It does this by walking JSON-tree edges (object members / array elements) and
  local `$ref` edges (JSON pointer references starting with `#`), with early
  exit as soon as the limit is exceeded.

  If the limit is exceeded (`JSON_SCHEMA_MAX_EXPANSION_DEPTH`),
  ER_JSON_DOCUMENT_TOO_DEEP is reported.

  @param schema_document Parsed JSON schema document.
  @retval true The schema exceeded the limit and an error was reported.
  @retval false No expansion path exceeds the limit.
*/
static bool check_json_schema_expansion_depth(
    const rapidjson::Document &schema_document) {
  std::unordered_set<const rapidjson::Value *> visiting;
  visiting.reserve(JSON_SCHEMA_MAX_EXPANSION_DEPTH);

  if (json_schema_exceeds_expansion_depth(
          schema_document, &schema_document, /*depth_so_far=*/0,
          JSON_SCHEMA_MAX_EXPANSION_DEPTH, &visiting)) {
    my_error(ER_JSON_DOCUMENT_TOO_DEEP, MYF(0));
    return true;
  }
  return false;
}

/**
  parse_json_schema will parse a JSON input into a JSON Schema. If the input
  isn't a valid JSON, or if the JSON is too deeply nested, an error will be
  returned to the user.

  @param json_schema_str A pointer to the JSON Schema input
  @param json_schema_length The length of the JSON Schema input
  @param function_name The function name of the caller (to be used in error
                       reporting)
  @param[out] schema_document An object where the JSON Schema will be put. This
              variable MUST be initialized.

  @retval true on error (my_error has been called)
  @retval false on success. The JSON Schema can be found in the output
          parameter schema_document.
*/
static bool parse_json_schema(const char *json_schema_str,
                              size_t json_schema_length,
                              const char *function_name,
                              rapidjson::Document *schema_document) {
  assert(schema_document != nullptr);

  // Check if the JSON schema is valid. Invalid JSON would be caught by
  // rapidjson::Document::Parse, but it will not catch documents that are too
  // deeply nested.
  size_t error_offset;
  std::string error_message;
  if (!is_valid_json_syntax(json_schema_str, json_schema_length, &error_offset,
                            &error_message, JsonDocumentDefaultDepthHandler)) {
    my_error(ER_INVALID_JSON_TEXT_IN_PARAM, MYF(0), 1, function_name,
             error_message.c_str(), error_offset, "");
    return true;
  }

  if (schema_document->Parse(json_schema_str, json_schema_length)
          .HasParseError()) {
    // The document should already be valid, since is_valid_json_syntax
    // succeeded.
    assert(false);
    return true;
  }

  // We require the JSON Schema to be an object
  if (!schema_document->IsObject()) {
    my_error(ER_INVALID_JSON_TYPE, MYF(0), 1, function_name, "object");
    return true;
  }

  return false;
}

bool is_valid_json_schema(const char *document_str, size_t document_length,
                          const char *json_schema_str,
                          size_t json_schema_length, const char *function_name,
                          bool *is_valid,
                          Json_schema_validation_report *validation_report) {
  rapidjson::Document schema_document;
  if (parse_json_schema(json_schema_str, json_schema_length, function_name,
                        &schema_document)) {
    return true;
  }

  if (check_json_schema_expansion_depth(schema_document)) {
    return true;
  }

  return Json_schema_validator(schema_document)
      .is_valid_json_schema(document_str, document_length, function_name,
                            is_valid, validation_report);
}

Json_schema_validator::Json_schema_validator(
    const rapidjson::Document &schema_document)
    : m_cached_schema(schema_document, /*uri=*/nullptr, /*uriLength=*/0,
                      &m_remote_document_provider) {}

unique_ptr_destroy_only<const Json_schema_validator>
create_json_schema_validator(MEM_ROOT *mem_root, const char *json_schema_str,
                             size_t json_schema_length,
                             const char *function_name) {
  rapidjson::Document schema_document;
  if (parse_json_schema(json_schema_str, json_schema_length, function_name,
                        &schema_document)) {
    return nullptr;
  }

  if (check_json_schema_expansion_depth(schema_document)) {
    return nullptr;
  }

  return make_unique_destroy_only<const Json_schema_validator>(mem_root,
                                                               schema_document);
}

bool Json_schema_validator::is_valid_json_schema(
    const char *document_str, size_t document_length, const char *function_name,
    bool *is_valid, Json_schema_validation_report *validation_report) const {
  // Set up the JSON Schema validator using Syntax_check_handler that will catch
  // JSON documents that are too deeply nested.
  Syntax_check_handler syntaxCheckHandler(JsonDocumentDefaultDepthHandler);
  rapidjson::GenericSchemaValidator<rapidjson::SchemaDocument,
                                    Syntax_check_handler>
      validator(m_cached_schema, syntaxCheckHandler);

  rapidjson::Reader reader;
  rapidjson::MemoryStream stream(document_str, document_length);

  // Wrap this in a try-catch since rapidjson calls std::regex_search
  // (which isn't noexcept).
  try {
    rapidjson::ParseResult parse_success = reader.Parse(stream, validator);
    // We may end up in a few different error scenarios here:
    // 1) The document is valid JSON, but invalid according to schema.
    //   - parse_success will indicate error, and validator.IsValid() is false.
    // 2) The JSON document is invalid (parsing failed), but not too deep.
    //   - parse_success will indicate error, and validator.IsValid() is true.
    // 3) The JSON document is too deep.
    //   - parse_success will indicate error, and validator.IsValid() is false.
    //     The only way do distinguish this from case 1, is to see if the
    //     syntax check handler has raised an error.
    if (syntaxCheckHandler.too_deep_error_raised()) {
      // The JSON document was too deep, and an error is already reported by the
      // Syntax_check_handler.
      return true;
    }

    if (!parse_success && validator.IsValid()) {
      // Couldn't parse the JSON document.
      std::pair<std::string, size_t> error = get_error_from_reader(reader);
      my_error(ER_INVALID_JSON_TEXT_IN_PARAM, MYF(0), 2, function_name,
               error.first.c_str(), error.second, "");
      return true;
    }

    // Otherwise, we have a syntactically correct JSON document, so we can
    // safely check the result from the validator.
  } catch (...) {
    handle_std_exception(function_name);
    return true;
  }

  // If we encountered a remote reference in the JSON schema, report an error
  // back to the user that this isn't supported.
  if (m_remote_document_provider.used()) {
    my_error(ER_NOT_SUPPORTED_YET, MYF(0), "references in JSON Schema");
    return true;
  }

  *is_valid = validator.IsValid();
  if (!validator.IsValid() && validation_report != nullptr) {
    // Populate the validation report. Since the validator is local to this
    // function, all strings provided by the validator must be allocated so
    // that they survive beyond this function.
    rapidjson::StringBuffer string_buffer;

    // Where in the JSON Schema the validation failed.
    validator.GetInvalidSchemaPointer().StringifyUriFragment(string_buffer);
    std::string schema_location(string_buffer.GetString(),
                                string_buffer.GetSize());

    // Where in the JSON document the validation failed.
    string_buffer.Clear();
    validator.GetInvalidDocumentPointer().StringifyUriFragment(string_buffer);
    std::string document_location(string_buffer.GetString(),
                                  string_buffer.GetSize());

    validation_report->set_error_report(std::move(schema_location),
                                        validator.GetInvalidSchemaKeyword(),
                                        std::move(document_location));
  }

  return false;
}

std::string Json_schema_validation_report::human_readable_reason() const {
  std::string reason;
  reason.append("The JSON document location '");
  reason.append(document_location());
  reason.append("' failed requirement '");
  reason.append(schema_failed_keyword());
  reason.append("' at JSON Schema location '");
  reason.append(schema_location());
  reason.append("'");
  return reason;
}
