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
#include <cassert>
#include <cctype>
#include <charconv>
#include <cstdlib>
#include <string>
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
}  // namespace

namespace storage::innobase::vec {

/* The shared implementation. Takes the two fields that actually matter -
the TYPE token and the WITH(...) list - so the same parse serves DDL,
where they arrive on a Key_spec, and table open, where they arrive on a
KEY. */
namespace {
/** Parse an integer WITH(...) value: the whole string, within [min, max].
@return true on error, reported through my_error() */
bool parse_int_option(const LEX_CSTRING &value, int min, int max, int *out) {
  const auto *last = value.str + value.length;
  int val;
  auto result = std::from_chars(value.str, last, val);
  if (result.ptr != last || result.ec != errc() || val < min || val > max) {
    my_error(ER_ILLEGAL_INDEX_CONSTRUCTION_PARAMETER_VALUE, MYF(0),
             value.str);
    return true;
  }
  *out = val;
  return false;
}

/** Copy a string onto a MEM_ROOT as a LEX_CSTRING.
@return false on OOM */
bool dup_lex_cstring(MEM_ROOT *mem_root, std::string_view s,
                     LEX_CSTRING *out) {
  out->str = strmake_root(mem_root, s.data(), s.size());
  out->length = s.size();
  return out->str != nullptr;
}
}  // namespace

bool parse_options(LEX_CSTRING type, const Vector_index_params_YY &params,
                   VectorIndexParam &vip) {
  namespace hnsw = vector_constants::hnsw;

  if (type.str == nullptr) {
    my_error(ER_NO_INDEX_TYPE, MYF(0), "");
    return true;
  }

  if (my_strcasecmp(system_charset_info, type.str, "HNSW") != 0) {
    my_error(ER_INDEX_TYPE_NOT_SUPPORTED, MYF(0), type.str, "vector");
    return true;
  }

  auto &hnsw_param = vip.emplace<HnswParam>();

  for (const auto &[key, value] : params) {
    if (my_strcasecmp(system_charset_info, key.str, "M") == 0) {
      if (parse_int_option(value, hnsw::min_M, hnsw::max_M, &hnsw_param.M))
        return true;
    } else if (my_strcasecmp(system_charset_info, key.str,
                             "ef_construction") == 0) {
      if (parse_int_option(value, hnsw::min_ef_construction,
                           hnsw::max_ef_construction,
                           &hnsw_param.ef_construction))
        return true;
    } else if (my_strcasecmp(system_charset_info, key.str, "metric") == 0) {
      std::string_view name(value.str, value.length);
      const auto *m = vector_constants::metric_from_name(name);
      if (m == nullptr) {
        my_error(ER_ILLEGAL_INDEX_CONSTRUCTION_PARAMETER_VALUE, MYF(0),
                 value.str);
        return true;
      }
      hnsw_param.metric = *m;
    } else {
      my_error(ER_ILLEGAL_INDEX_CONSTRUCTION_PARAMETER, MYF(0), key.str);
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

/* DDL-time: the one place the defaults turn into stored values. */
bool resolve_options(MEM_ROOT *mem_root, KEY *key) {
  VectorIndexParam vip;
  if (parse_options(*key, vip)) return true;
  const auto &hp = std::get<HnswParam>(vip);

  const std::string m = std::to_string(hp.M);
  const std::string efc = std::to_string(hp.ef_construction);
  const std::pair<std::string_view, std::string_view> resolved[] = {
      {"M", m},
      {"metric", vector_constants::metric_name(hp.metric)},
      {"ef_construction", efc},
  };

  /* A new list rather than an edit in place: the old one may be shared with
  the Key_spec, which a prepared statement executes again. */
  Vector_index_params_YY params;
  params.init(mem_root);
  for (const auto &[k, v] : resolved) {
    std::pair<LEX_CSTRING, LEX_CSTRING> param;
    if (!dup_lex_cstring(mem_root, k, &param.first) ||
        !dup_lex_cstring(mem_root, v, &param.second) ||
        params.push_back(param)) {
      return true; /* OOM, already reported by the mem_root */
    }
  }
  key->vector_index_params = params;
  return false;
}

}  // namespace storage::innobase::vec
