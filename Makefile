FETCH_CMD=wget
MASTER_SITE=http://www.percona.com/downloads/community
MYSQL_VERSION=5.5.9
PERCONA_SERVER ?=Percona-Server
DEBUG_DIR ?= $(PERCONA_SERVER)-debug
RELEASE_DIR ?= $(PERCONA_SERVER)-release
SERIES ?=series

CFLAGS=-fPIC -Wall -O3 -g -static-libgcc -fno-omit-frame-pointer -fno-strict-aliasing
CXXFLAGS=-fno-exceptions  -fPIC -Wall -Wno-unused-parameter -fno-implicit-templates -fno-exceptions -fno-rtti -O3 -g -static-libgcc -fno-omit-frame-pointer -fno-strict-aliasing

CFLAGS_RELEASE=$(CFLAGS) -DDBUG_OFF -DMY_PTHREAD_FASTMUTEX=1
CXXFLAGS_RELEASE=$(CXXFLAGS) -DDBUG_OFF -DMY_PTHREAD_FASTMUTEX=1

CMAKE=CC=gcc CXX=gcc cmake $(ADDITIONAL)
ADDITIONAL ?=
CONFIGURE=CFLAGS="-O2 -g -fmessage-length=0 -D_FORTIFY_SOURCE=2" CXXFLAGS="-O2 -g -fmessage-length=0 -D_FORTIFY_SOURCE=2"  LIBS=-lrt ./configure --prefix=/usr/local/$(PERCONA_SERVER)-$(MYSQL_VERSION) --with-plugin-innobase --with-plugin-partition

REVS = $(shell bzr log | grep rev | head -1   )
REV  = $(word 2, $(REVS) )

all: main install-lic tests misc
	@echo ""
	@echo "Percona Server source code is ready"

configure: all
	(cd $(PERCONA_SERVER); bash BUILD/autorun.sh; $(CONFIGURE))

cmake: cmake_release cmake_debug

cmake_release:
	rm -rf $(RELEASE_DIR)
	(mkdir -p $(RELEASE_DIR); cd $(RELEASE_DIR); CFLAGS="$(CFLAGS_RELEASE)" CXXFLAGS="$(CXXFLAGS_RELEASE)" $(CMAKE) -G "Unix Makefiles" ../$(PERCONA_SERVER))

cmake_debug:
	rm -rf $(DEBUG_DIR)
	(mkdir -p $(DEBUG_DIR); cd $(DEBUG_DIR); CFLAGS="$(CFLAGS)" CXXFLAGS="$(CXXFLAGS)" $(CMAKE) -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DWITH_DEBUG=ON -DMYSQL_MAINTAINER_MODE=OFF ../$(PERCONA_SERVER))

binary:
	(cd $(PERCONA_SERVER); CFLAGS="$(CFLAGS_RELEASE)" CXXFLAGS="$(CXXFLAGS_RELEASE)" ${CMAKE} . -DBUILD_CONFIG=mysql_release  \
           -DCMAKE_BUILD_TYPE=RelWithDebInfo \
	   -DCMAKE_INSTALL_PREFIX="/usr/local/$(PERCONA_SERVER)-$(MYSQL_VERSION)-$(REV)" \
           -DFEATURE_SET="community" \
	   -DWITH_EMBEDDED_SERVER=OFF \
           -DCOMPILATION_COMMENT="Percona-Server" \
           -DMYSQL_SERVER_SUFFIX="-$(REV)" )

install-lic: 
	@echo "Installing license files"
	install -m 644 COPYING.* $(PERCONA_SERVER)

main: mysql-$(MYSQL_VERSION).tar.gz
	@echo "Prepare Percona Server sources"
	rm -rf mysql-$(MYSQL_VERSION)
	rm -rf $(PERCONA_SERVER);
	tar zxf mysql-$(MYSQL_VERSION).tar.gz
	mv mysql-$(MYSQL_VERSION) $(PERCONA_SERVER)
	(cat `cat $(SERIES)`) | patch -p1 -d $(PERCONA_SERVER)
	rm $(PERCONA_SERVER)/sql/sql_yacc.cc $(PERCONA_SERVER)/sql/sql_yacc.h

mysql-$(MYSQL_VERSION).tar.gz:
	@echo "Downloading MySQL sources from $(MASTER_SITE)"
	$(FETCH_CMD) $(MASTER_SITE)/mysql-$(MYSQL_VERSION).tar.gz

tests:
	./install_tests.sh

misc:
	@echo "Installing other files"
	install -m 644 lrusort.py $(PERCONA_SERVER)/scripts

clean:
	rm -rf mysql-$(MYSQL_VERSION) $(PERCONA_SERVER)
	rm -f mysql-$(MYSQL_VERSION).tar.gz
