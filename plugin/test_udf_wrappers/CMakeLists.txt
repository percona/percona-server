MYSQL_ADD_PLUGIN(
  test_udf_wrappers
  test_udf_wrappers.cc
  MODULE_ONLY
  MODULE_OUTPUT_NAME "test_udf_wrappers"
)

set_target_properties(test_udf_wrappers PROPERTIES
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN YES
)

target_include_directories(test_udf_wrappers SYSTEM PRIVATE ${BOOST_PATCHES_DIR} ${BOOST_INCLUDE_DIR})
