AM_CPPFLAGS= $(MYSQL_INC)
AM_INCLUDES= -I. -I.. -I../libhsclient
ACLOCAL_AMFILES= $$ACLOCAL_ARGS
pkgplugindir=  $(PLUGIN_DIR)

noinst_HEADERS= database.hpp  hstcpsvr.hpp  hstcpsvr_worker.hpp  mysql_incl.hpp

pkgplugin_LTLIBRARIES       = handlersocket.la
#handlersocket_la_LDFLAGS= -shared -module -rpath $(pkgplugindir) 
handlersocket_la_LDFLAGS= -shared -module -rpath $(pkgplugindir) ../libhsclient/.libs/libhsclient.a
handlersocket_la_CFLAGS= $(AM_CFLAGS) $(AM_INCLUDES) 
handlersocket_la_CXXFLAGS= $(AM_CFLAGS) $(AM_INCLUDES) 
handlersocket_la_SOURCES= database.cpp handlersocket.cpp hstcpsvr_worker.cpp hstcpsvr.cpp

