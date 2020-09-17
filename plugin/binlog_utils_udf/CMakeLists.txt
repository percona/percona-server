MYSQL_ADD_PLUGIN(binlog_utils_udf
  binlog_utils_udf.cc
  MODULE_ONLY MODULE_OUTPUT_NAME "binlog_utils_udf"
  LINK_LIBRARIES extra::rapidjson
)

set_target_properties(binlog_utils_udf PROPERTIES
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN YES
)

target_compile_definitions(binlog_utils_udf PRIVATE MYSQL_SERVER)
target_include_directories(binlog_utils_udf SYSTEM PRIVATE ${BOOST_PATCHES_DIR} ${BOOST_INCLUDE_DIR})
