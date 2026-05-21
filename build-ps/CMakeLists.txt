# Generate percona-server.spec from template
# This file is processed by CMake's CONFIGURE_FILE with @ONLY mode,
# substituting @VARIABLE@ markers with CMake variable values.

IF(NOT LINUX)
  RETURN()
ENDIF()

# Derive Percona Server version from MYSQL_VERSION_EXTRA (strip leading dash)
STRING(REGEX REPLACE "^-" "" PERCONA_SERVER_VERSION "${MYSQL_VERSION_EXTRA}")

# Git revision for the spec
IF(NOT DEFINED PERCONA_REVISION)
  EXECUTE_PROCESS(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE PERCONA_REVISION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
  IF(NOT PERCONA_REVISION)
    SET(PERCONA_REVISION "unknown")
  ENDIF()
ENDIF()

# RPM release number
IF(NOT DEFINED RPM_RELEASE)
  SET(RPM_RELEASE "1")
ENDIF()

# Copyright year
STRING(TIMESTAMP MYSQL_COPYRIGHT_YEAR "%Y")

SET(SPECFILENAME "percona-server.spec")
CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/percona-server.spec.in
  ${CMAKE_CURRENT_BINARY_DIR}/${SPECFILENAME}
  @ONLY
)

MESSAGE(STATUS "Generated ${SPECFILENAME} with version ${MYSQL_NO_DASH_VERSION}-${PERCONA_SERVER_VERSION}")

# Generate DEB packaging files from templates
# TokuDB backup version
SET(TOKUDB_BACKUP_VERSION "${MYSQL_NO_DASH_VERSION}-${PERCONA_SERVER_VERSION}")

IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/debian/rules.in")
  CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/debian/rules.in
    ${CMAKE_CURRENT_BINARY_DIR}/debian/rules
    @ONLY
  )
  MESSAGE(STATUS "Generated debian/rules")
ENDIF()
