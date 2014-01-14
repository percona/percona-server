AM_INCLUDES= -I../libhsclient
bin_PROGRAMS=hsclient 
hsclient_SOURCES= hsclient.cpp
hsclient_LDFLAGS= -static -L../libhsclient -lhsclient
hsclient_CXXFLAGS= $(AM_INCLUDES)

hstest: hstest.o
	$(CXX) $(CXXFLAGS) $(LFLAGS) hstest.o \
		-L../libhsclient/.libs -lhsclient $$(mysql_config --libs_r) \
		-o hstest

hstest.o: hstest.cpp
	$(CXX) $(CXXFLAGS) $(AM_INCLUDES) $$(mysql_config --include) \
		-c hstest.cpp

hslongrun: hslongrun.o
	$(CXX) $(CXXFLAGS) $(LFLAGS) hslongrun.o \
		-L../libhsclient/.libs -lhsclient $$(mysql_config --libs_r) \
		-o hslongrun

hslongrun.o: hslongrun.cpp
	$(CXX) $(CXXFLAGS) $(AM_INCLUDES) $$(mysql_config --include) \
		-c hslongrun.cpp

