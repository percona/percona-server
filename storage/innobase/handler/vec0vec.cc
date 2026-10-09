/*****************************************************************************

Copyright (c) 2026, Percona Inc.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is designed to work with certain software (including
but not limited to OpenSSL) that is licensed under separate terms,
as designated in a particular file or component or in included license
documentation.  The authors of MySQL hereby grant you an additional
permission to link the program and your derivative works with the
separately licensed software that they have either included with
the program or referenced in the documentation.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

#include "vec0vec.h"

#include "vector-common/vector_distance.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cctype>
#include <charconv>
#include <cstdlib>
#include <type_traits>
#include <variant>

// ut0ut.h isn't self-contained.
#include "handler.h"
#include "lex_string.h"
#include "my_base.h"
#include "mysql/strings/m_ctype.h"
#include "mysqld_cs.h"
#include "ut0mem.h"
#include "ut0test.h"
#include "ut0ut.h"

#include <my_rapidjson_size_t.h>
#include <rapidjson/document.h>
#include <rapidjson/schema.h>
#include <rapidjson/stringbuffer.h>

#include "key_spec.h"
#include "my_sys.h"
#include "mysqld_error.h"

using namespace std;

namespace storage::innobase::vec {

namespace {

const char *alg_to_string(ha_key_alg alg) {
  switch (alg) {
    case HA_KEY_ALG_SE_SPECIFIC:
      ut_ad(0); /* the accepted algorithm; never named in an error */
      return "SE-SPECIFIC";
    case HA_KEY_ALG_BTREE:
      return "BTREE";
    case HA_KEY_ALG_RTREE:
      return "RTREE";
    case HA_KEY_ALG_HASH:
      return "HASH";
    case HA_KEY_ALG_FULLTEXT:
      return "FULLTEXT";
    case HA_KEY_ALG_VECTOR:
      return "VECTOR";
  }

  ut_ad(0); /* never nullptr: the caller passes this to my_error as %s */
  return "UNKNOWN";
}

/// Parses a value of type T out of a LEX_CSTRING. Returns false on failure.
template <typename T>
bool parse_value(const LEX_CSTRING &value, T &out) = delete;

template <>
inline bool parse_value<int>(const LEX_CSTRING &value, int &out) {
  const auto *last = value.str + value.length;
  auto result = std::from_chars(value.str, last, out);
  return result.ptr == last && result.ec == std::errc();
}

template <>
inline bool parse_value<vector_constants::Metric>(
    const LEX_CSTRING &value, vector_constants::Metric &out) {
  const auto *m = vector_constants::metric_from_name({value.str, value.length});
  if (m == nullptr) return false;
  out = *m;
  return true;
}

/**
  A single named option, mapping the name to a member of Param.
*/
template <typename ParamType, typename T>
struct Option {
  const char *name;
  T ParamType::*member;
  /// Optional extra validation for the option, e.g., range checks.
  std::type_identity_t<bool (*)(const T &)> validate = [](const T &) {
    return true;
  };

  [[nodiscard]] bool matches(const LEX_CSTRING &key) const {
    return my_strcasecmp(system_charset_info, key.str, name) == 0;
  }

  [[nodiscard]] bool parse(const LEX_CSTRING &value, ParamType &param) const {
    T parsed{};
    if (!parse_value<T>(value, parsed) || !validate(parsed)) return false;
    param.*member = parsed;
    return true;
  }
};

constexpr int kMinHnswM = 2;

/**
  Upper bound on the HNSW "M" option. Each node's neighbor buffer is
  (layer + 2) * M pointers (vector-common/hnsw.h), so an unbounded M turns
  even a single-row index into a multi-gigabyte allocation; 200 keeps that
  buffer small while leaving headroom over any M a real workload would
  choose.
*/
constexpr int kMaxHnswM = 200;

/// Template magic so that we don't have to explicitly add a new member to the
/// options variant for each option type
template <typename T, typename Variant>
struct variant_prepend_unique;
template <typename T, typename... Ts>
struct variant_prepend_unique<T, std::variant<Ts...>> {
  using type = std::conditional_t<(std::is_same_v<T, Ts> || ...),
                                  std::variant<Ts...>, std::variant<T, Ts...>>;
};

template <typename... Ts>
struct unique_variant {
  using type = std::variant<>;
};
template <typename T, typename... Ts>
struct unique_variant<T, Ts...> {
  using type = typename variant_prepend_unique<
      T, typename unique_variant<Ts...>::type>::type;
};

template <typename... Opts>
constexpr auto make_options(const Opts &...opts) {
  using Variant = typename unique_variant<Opts...>::type;
  return std::array<Variant, sizeof...(Opts)>{Variant{opts}...};
}

constexpr auto hnsw_options = make_options(
    Option{"M", &HnswParam::M,
           [](const int &v) { return v >= kMinHnswM && v <= kMaxHnswM; }},
    Option{"metric", &HnswParam::metric});

}  // namespace

/**
  The shared implementation. Takes the two fields that actually matter -
  the TYPE token and the (...) list - so the same parse serves DDL,
  where they arrive on a Key_spec, and table open, where they arrive on a
  KEY.
*/
bool parse_options(LEX_CSTRING type, const Vector_index_params_YY &params,
                   VectorIndexParam &vip) {
  if (type.str == nullptr) {
    my_error(ER_NO_INDEX_TYPE, MYF(0), "");
    return true;
  }

  if (my_strcasecmp(system_charset_info, type.str, "HNSW") != 0) {
    my_error(ER_INDEX_TYPE_NOT_SUPPORTED, MYF(0), type.str, "vector");
    return true;
  }

  auto &hnsw_param = vip.emplace<HnswParam>();

  std::array<bool, std::size(hnsw_options)> used_options{};
  for (const auto &[key, value] : params) {
    const auto it = std::find_if(
        std::begin(hnsw_options), std::end(hnsw_options),
        [&](const auto &option) {
          return std::visit([&](const auto &opt) { return opt.matches(key); },
                            option);
        });

    if (it == std::end(hnsw_options)) {
      my_error(ER_ILLEGAL_INDEX_CONSTRUCTION_PARAMETER, MYF(0), key.str);
      return true;
    }

    bool &is_used = used_options[std::distance(std::begin(hnsw_options), it)];
    if (is_used) {
      my_error(ER_DUPLICATE_INDEX_CONSTRUCTION_PARAMETER, MYF(0), key.str);
      return true;
    }
    is_used = true;

    if (!std::visit(
            [&](const auto &opt) { return opt.parse(value, hnsw_param); },
            *it)) {
      my_error(ER_ILLEGAL_INDEX_CONSTRUCTION_PARAMETER_VALUE, MYF(0),
               value.str);
      return true;
    }
  }

  if (hnsw_param.metric != vector_constants::Metric::kEuclidean) {
    my_error(ER_NOT_SUPPORTED_YET, MYF(0),
             "HNSW indexes on anything but the EUCLIDEAN metric");
    return true;
  }

  /* The only metric accepted. Squared euclidean is deliberate: the graph
  only ever compares distances, and skipping the square root costs nothing
  in ordering. */
  hnsw_param.dist = &vector_distance_euclidean_squared;
  return false;
}

bool validate_options(const Key_spec &index_def) {
  VectorIndexParam vip;
  return parse_options(index_def, vip);
}

/* DDL-time: validate the shape the user wrote, then parse. */
bool parse_options(const Key_spec &index_def, VectorIndexParam &vip) {
  if (index_def.type != KEYTYPE_VECTOR) return false;

  /* Column count is checked later by the server, not here. */
  ut_ad(!index_def.columns.empty());

  if (index_def.columns[0]->get_prefix_length() != 0) {
    my_error(ER_WRONG_SUB_KEY, MYF(0));
    return true;
  }

  if (index_def.key_create_info.algorithm != HA_KEY_ALG_SE_SPECIFIC) {
    my_error(ER_INDEX_TYPE_NOT_SUPPORTED, MYF(0),
             alg_to_string(index_def.key_create_info.algorithm), "vector");
    return true;
  }

  return parse_options(index_def.key_create_info.vector_index_type,
                       index_def.key_create_info.vector_index_params, vip);
}

/* Open-time: the definition came back from the DD, so its shape was
already validated at DDL time. Parse only. */
bool parse_options(const KEY &key, VectorIndexParam &vip) {
  return parse_options(key.vector_index_type, key.vector_index_params, vip);
}

}  // namespace storage::innobase::vec
