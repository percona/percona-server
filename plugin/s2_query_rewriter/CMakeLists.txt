
SET(S2_QREWRITER_SOURCES
#  s2/base/objs/strtoint.o
#  s2/base/objs/int128.o
#  s2/base/objs/stringprintf.o
#  s2/base/objs/logging.o
#  s2/util/coding/objs/coder.o
#  s2/util/coding/objs/varint.o
#  s2/util/math/objs/mathlimits.o
#  s2/util/math/objs/exactfloat.o
#  s2/util/math/objs/mathutil.o
#  s2/strings/objs/split.o
#  s2/strings/objs/ascii_ctype.o
#  s2/strings/objs/strutil.o
#  s2/strings/objs/stringprintf2.o
#  s2/objs/s1angle.o
#  s2/objs/s2.o
#  s2/objs/s2cellid.o
#  s2/objs/s2latlng.o
#  s2/objs/s1interval.o
#  s2/objs/s2cap.o
#  s2/objs/s2cell.o
#  s2/objs/s2cellunion.o
#  s2/objs/s2edgeindex.o
#  s2/objs/s2edgeutil.o
#  s2/objs/s2latlngrect.o
#  s2/objs/s2loop.o
#  s2/objs/s2pointregion.o
#  s2/objs/s2polygon.o
#  s2/objs/s2polygonbuilder.o
#  s2/objs/s2polyline.o
#  s2/objs/s2r2rect.o
#  s2/objs/s2region.o
#  s2/objs/s2regioncoverer.o
#  s2/objs/s2regionintersection.o
#  s2/objs/s2regionunion.o
  region_cover.cc
  plugin.cc)

MYSQL_ADD_PLUGIN(s2_query_rewriter ${S2_QREWRITER_SOURCES}
  MODULE_ONLY MODULE_OUTPUT_NAME "s2_query_rewriter")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/plugin/s2_query_rewriter/s2)
#TARGET_LINK_LIBRARIES(s2_query_rewriter ${CMAKE_SOURCE_DIR}/plugin/s2_query_rewriter/libs2_all.a)
TARGET_LINK_LIBRARIES(s2_query_rewriter pthread)
TARGET_LINK_LIBRARIES(s2_query_rewriter crypto)
TARGET_LINK_LIBRARIES(s2_query_rewriter gflags)
TARGET_LINK_LIBRARIES(s2_query_rewriter s2)
SET(CMAKE_CXX_FLAGS "-std=c++11 -Wall ${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs -Wno-ignored-qualifiers -Wno-unused-variable -Wno-unused-function")
##SET_TARGET_PROPERTIES(s2_query_rewriter PROPERTIES LINK_FLAGS -Wl,-Wno-unused-local-typedefs)
##SET_TARGET_PROPERTIES(s2_query_rewriter PROPERTIES CXXFLAGS -Wl,-Wno-unused-local-typedefs)