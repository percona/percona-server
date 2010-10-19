AM_INCLUDES= -I. -I.. -I../libhsclient
ACLOCAL_AMFILES= $$ACLOCAL_ARGS
pkgplugindir=  $(PLUGIN_DIR)

bin_PROGRAMS=hsclient 
hsclient_SOURCES= hsclient.cpp
hsclient_LDFLAGS= -static -L../libhsclient/.libs -lhsclient
hsclient_CXXFLAGS= $(AM_INCLUDES)

hstest: hstest.o
	$(CXX) $(CXXFLAGS) $(LFLAGS) hstest.o -L../libhsclient/.libs -lhsclient $(MYSQL_LIB) -o hstest
hstest.o: hstest.cpp
	$(CXX) $(CXXFLAGS) $(MYSQL_INC) $(AM_INCLUDES) -c hstest.cpp

