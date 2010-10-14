AM_CPPFLAGS= $(MYSQL_INC)
AM_INCLUDES= -I. -I.. -I../libhsclient
AM_LIBS= $(MYSQL_LIB)
ACLOCAL_AMFILES= $$ACLOCAL_ARGS
pkgplugindir=  $(PLUGIN_DIR)

bin_PROGRAMS=hsclient hstest
hsclient_SOURCES= hsclient.cpp
#hsclient_LDADD= -L../libhsclient/.libs -lhsclient $(AM_LIBS) -lmysqlclient -lm -lpthread
hsclient_LDFLAGS= -static -L../libhsclient/.libs -lhsclient $(AM_LIBS) -lmysqlclient -lm -lpthread
hsclient_CXXFLAGS= $(AM_CFLAGS) $(AM_INCLUDES)

hstest_SOURCES= hstest.cpp
#hstest_LDADD= -L../libhsclient/.libs -lhsclient $(AM_LIBS) -lmysqlclient -lm -lpthread
hstest_LDFLAGS= -static -L../libhsclient/.libs -lhsclient $(AM_LIBS) -lmysqlclient -lm -lpthread
hstest_CXXFLAGS= $(AM_CFLAGS) $(AM_INCLUDES)

