MYSQL_ADD_PLUGIN(
  test_udf_wrappers
  test_udf_wrappers.cc
  MODULE_ONLY
  MODULE_OUTPUT_NAME "test_udf_wrappers"
)

INCLUDE_DIRECTORIES(SYSTEM ${BOOST_PATCHES_DIR} ${BOOST_INCLUDE_DIR})
