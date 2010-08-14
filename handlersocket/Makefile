
ifndef MYSQL_INC
MYSQL_INC = /usr/include/mysql
endif

ifndef MYSQL_VERSION
MYSQL_VERSION = $(shell ./get_mysql_version.sh $(MYSQL_INC))
endif

ifndef MYSQL_SRC
MYSQL_SRC = ./mysql-hdrs-$(MYSQL_VERSION)
endif

CXX = g++ -Wall -g -fno-rtti -fno-exceptions -fPIC -DPIC
LIBS = -lhsclient -lpthread -lz
CXXFLAGS = -I/usr/include/handlersocket \
	-I$(MYSQL_INC) \
	-I$(MYSQL_SRC)/sql \
	-I$(MYSQL_SRC)/include \
	-I$(MYSQL_SRC)/regex
LDFLAGS =

# CXXFLAGS += -O0
CXXFLAGS += -O3 -DNDEBUG
#CXXFLAGS += -fprofile-arcs -ftest-coverage

HANDLERSOCKET_OBJS = database.o hstcpsvr.o hstcpsvr_worker.o

all: check_src handlersocket.so

check_src:
	if [ ! -d $(MYSQL_SRC) ]; then \
		echo "$(MYSQL_SRC) is missing\n"; \
		exit 1; \
	fi

handlersocket.so: $(HANDLERSOCKET_OBJS) handlersocket.cpp
	$(CXX) $(CXXFLAGS) -fno-strict-aliasing -shared $^ -o $@ $(LDFLAGS) \
		-Wl,-soname -Wl,$@ $(LIBS)
clean:
	rm -f *.a *.so *.o

LIBDIR = $(shell \
  if [ -e /usr/lib64/mysql ]; then echo /usr/lib64; else echo /usr/lib; fi)

install: handlersocket.so
	sudo sh -c 'ulimit -c unlimited ; /etc/init.d/mysql stop ; \
		cp handlersocket.so handlersocket.so.cpy && \
		mv handlersocket.so.cpy \
			$(LIBDIR)/mysql/plugin/handlersocket.so && \
		/etc/init.d/mysql start'

