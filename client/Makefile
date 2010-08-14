
CXX = g++ -Wall -g -fno-rtti -fno-exceptions -fPIC -DPIC
LIBS = -lhsclient -lpthread -lz -lrt
LDFLAGS =

# CXXFLAGS += -O0
CXXFLAGS += -O3 -DNDEBUG
#CXXFLAGS += -fprofile-arcs -ftest-coverage
CXXFLAGS += -I/usr/include/handlersocket

all: hsclient hstest

hsclient: hsclient.o
	$(CXX) $(CXXFLAGS) -I/usr/include/mysql -o $@ $^ \
		-L/usr/lib64/mysql -L/usr/lib/mysql -lmysqlclient_r $(LIBS)

hstest: hstest.cpp
	$(CXX) $(CXXFLAGS) -I/usr/include/mysql -o $@ $^ \
		-L/usr/lib64/mysql -L/usr/lib/mysql -lmysqlclient_r $(LIBS)

clean:
	rm -f *.a *.so *.o hsclient hstest

