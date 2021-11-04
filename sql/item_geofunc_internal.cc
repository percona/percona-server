/* Copyright (c) 2015, 2021, Oracle and/or its affiliates.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "sql/item_geofunc_internal.h"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <memory>

#include <boost/concept/usage.hpp>
#include <boost/geometry/algorithms/centroid.hpp>
#include <boost/geometry/algorithms/is_valid.hpp>
#include <boost/geometry/algorithms/overlaps.hpp>
#include <boost/geometry/core/exception.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/predicates.hpp>
#include <boost/geometry/strategies/strategies.hpp>
#include <boost/iterator/iterator_facade.hpp>

#include "m_ctype.h"
#include "m_string.h"
#include "my_byteorder.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "sql/current_thd.h"
#include "sql/dd/cache/dictionary_client.h"
#include "sql/item_func.h"
#include "sql/mdl.h"
#include "sql/parse_location.h"  // POS
#include "sql/sql_class.h"       // THD
#include "sql/srs_fetcher.h"
#include "sql/system_variables.h"
#include "sql_string.h"
#include "template_utils.h"

namespace dd {
class Spatial_reference_system;
}  // namespace dd

bool Srs_fetcher::lock(gis::srid_t srid, enum_mdl_type lock_type) {
  DBUG_TRACE;
  assert(srid != 0);

  char id_str[11];  // uint32 => max 10 digits + \0
  longlong10_to_str(srid, id_str, 10);

  MDL_request mdl_request;
  mdl_request.init_with_source(MDL_key::SRID, "", id_str, lock_type,
                               MDL_TRANSACTION, __FILE__, __LINE__);
  if (m_thd->mdl_context.acquire_lock(&mdl_request,
                                      m_thd->variables.lock_wait_timeout)) {
    /* purecov: begin inspected */
    // If locking fails, an error has already been flagged.
    return true;
    /* purecov: end */
  }

  return false;
}

bool Srs_fetcher::acquire(gis::srid_t srid,
                          const dd::Spatial_reference_system **srs) {
  if (lock(srid, MDL_SHARED_READ)) return true; /* purecov: inspected */

  if (m_thd->dd_client()->acquire(srid, srs))
    return true; /* purecov: inspected */
  return false;
}

bool Srs_fetcher::acquire_for_modification(gis::srid_t srid,
                                           dd::Spatial_reference_system **srs) {
  if (lock(srid, MDL_EXCLUSIVE)) return true; /* purecov: inspected */

  if (m_thd->dd_client()->acquire_for_modification(srid, srs))
    return true; /* purecov: inspected */
  return false;
}

bool Srs_fetcher::srs_exists(THD *thd, gis::srid_t srid, bool *exists) {
  assert(exists);
  std::unique_ptr<dd::cache::Dictionary_client::Auto_releaser> releaser(
      new dd::cache::Dictionary_client::Auto_releaser(thd->dd_client()));
  Srs_fetcher fetcher(thd);
  const dd::Spatial_reference_system *srs = nullptr;
  if (fetcher.acquire(srid, &srs)) return true; /* purecov: inspected */
  *exists = (srs != nullptr);
  return false;
}

<<<<<<< HEAD
template <typename Coordsys>
void BG_geometry_collection::merge_components(bool *pnull_value) {
  if (is_comp_no_overlapped()) return;

  Item_func_st_union ifsu(POS{{nullptr, nullptr}, {nullptr, nullptr}}, nullptr, nullptr);
  bool do_again = true;
  uint32 last_composition[6] = {0}, num_unchanged_composition = 0;
  size_t last_num_geos = 0;

  /*
    After each merge_one_run call, see whether the two indicators change:
    1. total number of geometry components;
    2. total number of each of the 6 types of geometries

    If they don't change for N*N/4 times, break out of the loop. Here N is
    the total number of geometry components.

    There is the rationale:

    Given a geometry collection, it's likely that one effective merge_one_run
    merges a polygon P and the linestring that crosses it (L) to a
    polygon P'(the same one) and another linestring L', the 2 indicators above
    don't change but the merge is actually done. If we merge P'
    and L' again, they should not be considered cross, but given certain data
    BG somehow believes L` still crosses P` even the P and P` are valid, and
    it will give us a L'' and P'' which is different from L' and P'
    respectively, and L'' is still considered crossing P'',
    hence the loop here never breaks out.

    If the collection has N components, and we have X [multi]linestrings and
    N-X polygons, the number of pairs that can be merged is Y = X * (N-X),
    so the largest Y is N*N/4. If the above 2 indicators stay unchanged more
    than N*N/4 times the loop runs, we believe all possible combinations in
    the collection are enumerated and no effective merge is being done any more.

    Note that the L'' and P'' above is different from L' and P' so we can't
    compare GEOMETRY byte string, and geometric comparison is expensive and may
    still compare unequal and we would still be stuck in the endless loop.
  */
  while (!*pnull_value && do_again) {
    do_again = merge_one_run<Coordsys>(&ifsu, pnull_value);
    if (!*pnull_value && do_again) {
      const size_t num_geos = m_geos.size();
      uint32 composition[6] = {0};

      for (size_t i = 0; i < num_geos; ++i)
        composition[m_geos[i]->get_type() - 1]++;

      if (num_geos != last_num_geos ||
          memcmp(composition, last_composition, sizeof(composition))) {
        memcpy(last_composition, composition, sizeof(composition));
        last_num_geos = num_geos;
        num_unchanged_composition = 0;
      } else
        num_unchanged_composition++;

      if (num_unchanged_composition > (last_num_geos * last_num_geos / 4 + 2))
        break;
    }
  }
}

// Explicit template instantiation
template void BG_geometry_collection::merge_components<
    boost::geometry::cs::cartesian>(bool *);

template <typename Coordsys>
inline bool linestring_overlaps_polygon_outerring(const Gis_line_string &ls,
                                                  const Gis_polygon &plgn) {
  Gis_polygon_ring &oring = plgn.outer();
  Gis_line_string ls2(oring.get_ptr(), oring.get_nbytes(), oring.get_flags(),
                      oring.get_srid());
  return boost::geometry::overlaps(ls, ls2);
}

template <typename Coordsys>
bool linear_areal_intersect_infinite(Geometry *g1, Geometry *g2,
                                     bool *pnull_value) {
  bool res = false;

  /*
    If crosses check succeeds, make sure g2 is a valid [multi]polygon, invalid
    ones can be accepted by BG and the cross check would be considered true,
    we should reject such result and return false in this case.
  */
  if (Item_func_spatial_rel::bg_geo_relation_check(
          g1, g2, Item_func::SP_CROSSES_FUNC, pnull_value) &&
      !*pnull_value) {
    Geometry::wkbType g2_type = g2->get_type();
    if (g2_type == Geometry::wkb_polygon) {
      Gis_polygon plgn(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                       g2->get_srid());
      res = bg::is_valid(plgn);
    } else if (g2_type == Geometry::wkb_multipolygon) {
      Gis_multi_polygon mplgn(g2->get_data_ptr(), g2->get_data_size(),
                              g2->get_flags(), g2->get_srid());
      res = bg::is_valid(mplgn);
    } else
      assert(false);

    return res;
  }

  if (*pnull_value) return false;

  if (g1->get_type() == Geometry::wkb_linestring) {
    Gis_line_string ls(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                       g1->get_srid());
    if (g2->get_type() == Geometry::wkb_polygon) {
      Gis_polygon plgn(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                       g2->get_srid());
      res = linestring_overlaps_polygon_outerring<Coordsys>(ls, plgn);
    } else {
      Gis_multi_polygon mplgn(g2->get_data_ptr(), g2->get_data_size(),
                              g2->get_flags(), g2->get_srid());
      for (size_t i = 0; i < mplgn.size(); i++) {
        if (linestring_overlaps_polygon_outerring<Coordsys>(ls, mplgn[i]))
          return true;
      }
    }
  } else {
    Gis_multi_line_string mls(g1->get_data_ptr(), g1->get_data_size(),
                              g1->get_flags(), g1->get_srid());
    if (g2->get_type() == Geometry::wkb_polygon) {
      Gis_polygon plgn(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                       g2->get_srid());
      for (size_t i = 0; i < mls.size(); i++) {
        if (linestring_overlaps_polygon_outerring<Coordsys>(mls[i], plgn))
          return true;
      }
    } else {
      Gis_multi_polygon mplgn(g2->get_data_ptr(), g2->get_data_size(),
                              g2->get_flags(), g2->get_srid());
      for (size_t j = 0; j < mls.size(); j++) {
        for (size_t i = 0; i < mplgn.size(); i++) {
          if (linestring_overlaps_polygon_outerring<Coordsys>(mls[j], mplgn[i]))
            return true;
        }
      }
    }
  }

  return res;
}

||||||| beb865a960b
template <typename Coordsys>
void BG_geometry_collection::merge_components(bool *pnull_value) {
  if (is_comp_no_overlapped()) return;

  POS pos{{nullptr, nullptr}, {nullptr, nullptr}};
  Item_func_st_union ifsu(pos, nullptr, nullptr);
  bool do_again = true;
  uint32 last_composition[6] = {0}, num_unchanged_composition = 0;
  size_t last_num_geos = 0;

  /*
    After each merge_one_run call, see whether the two indicators change:
    1. total number of geometry components;
    2. total number of each of the 6 types of geometries

    If they don't change for N*N/4 times, break out of the loop. Here N is
    the total number of geometry components.

    There is the rationale:

    Given a geometry collection, it's likely that one effective merge_one_run
    merges a polygon P and the linestring that crosses it (L) to a
    polygon P'(the same one) and another linestring L', the 2 indicators above
    don't change but the merge is actually done. If we merge P'
    and L' again, they should not be considered cross, but given certain data
    BG somehow believes L` still crosses P` even the P and P` are valid, and
    it will give us a L'' and P'' which is different from L' and P'
    respectively, and L'' is still considered crossing P'',
    hence the loop here never breaks out.

    If the collection has N components, and we have X [multi]linestrings and
    N-X polygons, the number of pairs that can be merged is Y = X * (N-X),
    so the largest Y is N*N/4. If the above 2 indicators stay unchanged more
    than N*N/4 times the loop runs, we believe all possible combinations in
    the collection are enumerated and no effective merge is being done any more.

    Note that the L'' and P'' above is different from L' and P' so we can't
    compare GEOMETRY byte string, and geometric comparison is expensive and may
    still compare unequal and we would still be stuck in the endless loop.
  */
  while (!*pnull_value && do_again) {
    do_again = merge_one_run<Coordsys>(&ifsu, pnull_value);
    if (!*pnull_value && do_again) {
      const size_t num_geos = m_geos.size();
      uint32 composition[6] = {0};

      for (size_t i = 0; i < num_geos; ++i)
        composition[m_geos[i]->get_type() - 1]++;

      if (num_geos != last_num_geos ||
          memcmp(composition, last_composition, sizeof(composition))) {
        memcpy(last_composition, composition, sizeof(composition));
        last_num_geos = num_geos;
        num_unchanged_composition = 0;
      } else
        num_unchanged_composition++;

      if (num_unchanged_composition > (last_num_geos * last_num_geos / 4 + 2))
        break;
    }
  }
}

// Explicit template instantiation
template void BG_geometry_collection::merge_components<
    boost::geometry::cs::cartesian>(bool *);

template <typename Coordsys>
inline bool linestring_overlaps_polygon_outerring(const Gis_line_string &ls,
                                                  const Gis_polygon &plgn) {
  Gis_polygon_ring &oring = plgn.outer();
  Gis_line_string ls2(oring.get_ptr(), oring.get_nbytes(), oring.get_flags(),
                      oring.get_srid());
  return boost::geometry::overlaps(ls, ls2);
}

template <typename Coordsys>
bool linear_areal_intersect_infinite(Geometry *g1, Geometry *g2,
                                     bool *pnull_value) {
  bool res = false;

  /*
    If crosses check succeeds, make sure g2 is a valid [multi]polygon, invalid
    ones can be accepted by BG and the cross check would be considered true,
    we should reject such result and return false in this case.
  */
  if (Item_func_spatial_rel::bg_geo_relation_check(
          g1, g2, Item_func::SP_CROSSES_FUNC, pnull_value) &&
      !*pnull_value) {
    Geometry::wkbType g2_type = g2->get_type();
    if (g2_type == Geometry::wkb_polygon) {
      Gis_polygon plgn(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                       g2->get_srid());
      res = bg::is_valid(plgn);
    } else if (g2_type == Geometry::wkb_multipolygon) {
      Gis_multi_polygon mplgn(g2->get_data_ptr(), g2->get_data_size(),
                              g2->get_flags(), g2->get_srid());
      res = bg::is_valid(mplgn);
    } else
      assert(false);

    return res;
  }

  if (*pnull_value) return false;

  if (g1->get_type() == Geometry::wkb_linestring) {
    Gis_line_string ls(g1->get_data_ptr(), g1->get_data_size(), g1->get_flags(),
                       g1->get_srid());
    if (g2->get_type() == Geometry::wkb_polygon) {
      Gis_polygon plgn(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                       g2->get_srid());
      res = linestring_overlaps_polygon_outerring<Coordsys>(ls, plgn);
    } else {
      Gis_multi_polygon mplgn(g2->get_data_ptr(), g2->get_data_size(),
                              g2->get_flags(), g2->get_srid());
      for (size_t i = 0; i < mplgn.size(); i++) {
        if (linestring_overlaps_polygon_outerring<Coordsys>(ls, mplgn[i]))
          return true;
      }
    }
  } else {
    Gis_multi_line_string mls(g1->get_data_ptr(), g1->get_data_size(),
                              g1->get_flags(), g1->get_srid());
    if (g2->get_type() == Geometry::wkb_polygon) {
      Gis_polygon plgn(g2->get_data_ptr(), g2->get_data_size(), g2->get_flags(),
                       g2->get_srid());
      for (size_t i = 0; i < mls.size(); i++) {
        if (linestring_overlaps_polygon_outerring<Coordsys>(mls[i], plgn))
          return true;
      }
    } else {
      Gis_multi_polygon mplgn(g2->get_data_ptr(), g2->get_data_size(),
                              g2->get_flags(), g2->get_srid());
      for (size_t j = 0; j < mls.size(); j++) {
        for (size_t i = 0; i < mplgn.size(); i++) {
          if (linestring_overlaps_polygon_outerring<Coordsys>(mls[j], mplgn[i]))
            return true;
        }
      }
    }
  }

  return res;
}

=======
>>>>>>> mysql-8.0.27
/**
  Create this class for exception safety --- destroy the objects referenced
  by the pointers in the set when destroying the container.
 */
template <typename T>
class Pointer_vector : public std::vector<T *> {
  typedef std::vector<T *> parent;

 public:
  ~Pointer_vector() {
    for (typename parent::iterator i = this->begin(); i != this->end(); ++i)
      delete (*i);
  }
};

// A unary predicate to locate a target Geometry object pointer from a sequence.
class Is_target_geometry {
  Geometry *m_target;

 public:
  Is_target_geometry(Geometry *t) : m_target(t) {}

  bool operator()(Geometry *g) { return g == m_target; }
};

class Rtree_entry_compare {
 public:
  Rtree_entry_compare() = default;

  bool operator()(const BG_rtree_entry &re1, const BG_rtree_entry &re2) const {
    return re1.second < re2.second;
  }
};

inline void reassemble_geometry(Geometry *g) {
  Geometry::wkbType gtype = g->get_geotype();
  if (gtype == Geometry::wkb_polygon)
    down_cast<Gis_polygon *>(g)->to_wkb_unparsed();
  else if (gtype == Geometry::wkb_multilinestring)
    down_cast<Gis_multi_line_string *>(g)->reassemble();
  else if (gtype == Geometry::wkb_multipolygon)
    down_cast<Gis_multi_polygon *>(g)->reassemble();
}

template <typename BG_geotype>
bool post_fix_result(BG_result_buf_mgr *resbuf_mgr, BG_geotype &geout,
                     String *res) {
  assert(geout.has_geom_header_space());
  reassemble_geometry(&geout);

  // Such objects returned by BG never have overlapped components.
  if (geout.get_type() == Geometry::wkb_multilinestring ||
      geout.get_type() == Geometry::wkb_multipolygon)
    geout.set_components_no_overlapped(true);
  if (geout.get_ptr() == nullptr) return true;
  if (res) {
    char *resptr = geout.get_cptr() - GEOM_HEADER_SIZE;
    size_t len = geout.get_nbytes();

    /*
      The resptr buffer is now owned by resbuf_mgr and used by res, resptr
      will be released properly by resbuf_mgr.
     */
    resbuf_mgr->add_buffer(resptr);
    /*
      The memory for the result is owned by a BG_result_buf_mgr,
      so use String::set(char*, size_t, const CHARSET_INFO)
      which points the internall buffer to the input argument,
      and sets m_is_alloced = false, signifying the String object
      does not own the buffer.
    */
    res->set(resptr, len + GEOM_HEADER_SIZE, &my_charset_bin);

    // Prefix the GEOMETRY header.
    write_geometry_header(resptr, geout.get_srid(), geout.get_geotype());

    /*
      Give up ownership because the buffer may have to live longer than
      the object.
    */
    geout.set_ownmem(false);
  }

  return false;
}

// Explicit template instantiation
template bool post_fix_result<Gis_line_string>(BG_result_buf_mgr *,
                                               Gis_line_string &, String *);
template bool post_fix_result<Gis_multi_line_string>(BG_result_buf_mgr *,
                                                     Gis_multi_line_string &,
                                                     String *);
template bool post_fix_result<Gis_multi_point>(BG_result_buf_mgr *,
                                               Gis_multi_point &, String *);
template bool post_fix_result<Gis_multi_polygon>(BG_result_buf_mgr *,
                                                 Gis_multi_polygon &, String *);
template bool post_fix_result<Gis_point>(BG_result_buf_mgr *, Gis_point &,
                                         String *);
template bool post_fix_result<Gis_polygon>(BG_result_buf_mgr *, Gis_polygon &,
                                           String *);

class Is_empty_geometry : public WKB_scanner_event_handler {
 public:
  bool is_empty;

  Is_empty_geometry() : is_empty(true) {}

  void on_wkb_start(Geometry::wkbByteOrder, Geometry::wkbType geotype,
                    const void *, uint32, bool) override {
    if (is_empty && geotype != Geometry::wkb_geometrycollection)
      is_empty = false;
  }

  void on_wkb_end(const void *) override {}

  bool continue_scan() const override { return is_empty; }
};

bool is_empty_geocollection(const Geometry *g) {
  if (g->get_geotype() != Geometry::wkb_geometrycollection) return false;

  uint32 num = uint4korr(g->get_cptr());
  if (num == 0) return true;

  Is_empty_geometry checker;
  uint32 len = g->get_data_size();
  wkb_scanner(current_thd, g->get_cptr(), &len,
              Geometry::wkb_geometrycollection, false, &checker);
  return checker.is_empty;
}

bool is_empty_geocollection(const String &wkbres) {
  if (wkbres.ptr() == nullptr) return true;

  uint32 geotype = uint4korr(wkbres.ptr() + SRID_SIZE + 1);

  if (geotype != static_cast<uint32>(Geometry::wkb_geometrycollection))
    return false;

  if (uint4korr(wkbres.ptr() + SRID_SIZE + WKB_HEADER_SIZE) == 0) return true;

  Is_empty_geometry checker;
  uint32 len = static_cast<uint32>(wkbres.length()) - GEOM_HEADER_SIZE;
  wkb_scanner(current_thd, wkbres.ptr() + GEOM_HEADER_SIZE, &len,
              Geometry::wkb_geometrycollection, false, &checker);
  return checker.is_empty;
}
