IF(CMAKE_CXX_FLAGS)
  STRING(REPLACE "-fno-implicit-templates" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
ENDIF()
ADD_DEFINITIONS(-fno-rtti -fno-exceptions -fPIC -DPIC)
ADD_SUBDIRECTORY(libhsclient)
INCLUDE_DIRECTORIES(libhsclient)
SET ( HANDLERSOCKET_SOURCES
    handlersocket/database.cpp handlersocket/handlersocket.cpp handlersocket/hstcpsvr_worker.cpp handlersocket/hstcpsvr.cpp
)

MYSQL_ADD_PLUGIN(handlersocket ${HANDLERSOCKET_SOURCES} LINK_LIBRARIES hsclient MODULE_ONLY)
