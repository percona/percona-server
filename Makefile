FETCH_CMD=wget
MASTER_SITE=http://www.percona.com/downloads/community
MYSQL_VERSION=5.5.8
PERCONA_SERVER ?=Percona-Server
DEBUG_DIR ?= $(PERCONA_SERVER)-debug
RELEASE_DIR ?= $(PERCONA_SERVER)-release
SERIES ?=series
CMAKE=CC=/usr/bin/gcc CXX=/usr/bin/gcc CFLAGS="-fPIC -Wall -O3 -g -static-libgcc -fno-omit-frame-pointer -fno-strict-aliasing -DDBUG_OFF" CXXFLAGS="-fno-exceptions  -fPIC -Wall -Wno-unused-parameter -fno-implicit-templates -fno-exceptions -fno-rtti -O3 -g -static-libgcc -fno-omit-frame-pointer -fno-strict-aliasing -DDBUG_OFF" cmake 
CONFIGUR=CFLAGS="-O2 -g -fmessage-length=0 -D_FORTIFY_SOURCE=2" CXXFLAGS="-O2 -g -fmessage-length=0 -D_FORTIFY_SOURCE=2"  LIBS=-lrt ./configure --prefix=/usr/local/$(PERCONA_SERVER)-$(MYSQL_VERSION) --with-plugin-innobase --with-plugin-partition


all: main install-lic tests misc
	@echo ""
	@echo "Percona Server source code is ready"
	@echo "Now change directory to $(PERCONA_SERVER) define variables as show below"
	@echo ""
	export CFLAGS="-O2 -g -fmessage-length=0 -D_FORTIFY_SOURCE=2"
	export CXXFLAGS="-O2 -g -fmessage-length=0 -D_FORTIFY_SOURCE=2"
	export LIBS=-lrt
	@echo ""
	@echo "and run ./configure --prefix=/usr/local/$(PERCONA_SERVER)-$(MYSQL_VERSION) --with-plugin-innobase --with-plugin-partition && make all install"
	@echo ""

configure: all
	(cd $(PERCONA_SERVER); bash BUILD/autorun.sh; $(CONFIGUR))

cmake:
	rm -rf $(DEBUG_DIR)
	rm -rf $(RELEASE_DIR)
	(mkdir -p $(DEBUG_DIR); cd $(DEBUG_DIR); $(CMAKE) -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DWITH_DEBUG=Full -DMYSQL_MAINTAINER_MODE=OFF ../$(PERCONA_SERVER))
	(mkdir -p $(RELEASE_DIR); cd $(RELEASE_DIR); $(CMAKE) -G "Unix Makefiles" ../$(PERCONA_SERVER))

binary:
	(cd $(PERCONA_SERVER); ${CMAKE} . -DBUILD_CONFIG=mysql_release  \
           -DCMAKE_BUILD_TYPE=RelWithDebInfo \
	   -DCMAKE_INSTALL_PREFIX="/usr/local/$(PERCONA_SERVER)-$(MYSQL_VERSION)" \
           -DFEATURE_SET="community" \
	   -DWITH_EMBEDDED_SERVER=OFF \
           -DCOMPILATION_COMMENT="Percona-Server" \
           -DMYSQL_SERVER_SUFFIX="${MYSQL_VERSION}" )

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

clean: env
	rm -rf mysql-$(MYSQL_VERSION) $(PERCONA_SERVER)
	rm -f mysql-$(MYSQL_VERSION).tar.gz
