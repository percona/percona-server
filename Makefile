FETCH_CMD=wget
MASTER_SITE=http://www.percona.com/downloads/community
MYSQL_VERSION=5.5.7-rc
PERCONA_SERVER ?=Percona-Server
DEBUG_DIR ?= $(PERCONA_SERVER)-debug
RELEASE_DIR ?= $(PERCONA_SERVER)-release

all: main install-lic tests misc
	@echo ""
	@echo "Percona Server source code is ready"
	@echo "Now change directory to $(PERCONA_SERVER) define variables as show below"
	@echo ""
	export CFLAGS="-O2 -g -fmessage-length=0 -D_FORTIFY_SOURCE=2"
	export CXXFLAGS="-O2 -g -fmessage-length=0 -D_FORTIFY_SOURCE=2"
	export LIBS=-lrt
	@echo ""
	@echo "and run ./configure ... --without-plugin-innobase --with-plugin-innodb_plugin && make all install"
	@echo ""

cmake:
	rm -rf $(DEBUG_DIR)
	rm -rf $(RELEASE_DIR)
	(mkdir -p $(DEBUG_DIR); cd $(DEBUG_DIR); cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug -DWITH_DEBUG=Full ../$(PERCONA_SERVER))
	(mkdir -p $(RELEASE_DIR); cd $(RELEASE_DIR); cmake -G "Unix Makefiles" ../$(PERCONA_SERVER))

install-lic: 
	@echo "Installing license files"
	install -m 644 COPYING.* $(PERCONA_SERVER)

main: mysql-$(MYSQL_VERSION).tar.gz
	@echo "Prepare Percona Server sources"
	rm -rf mysql-$(MYSQL_VERSION)
	rm -rf $(PERCONA_SERVER);
	tar zxf mysql-$(MYSQL_VERSION).tar.gz
	mv mysql-$(MYSQL_VERSION) $(PERCONA_SERVER)
	(cat `cat series`) | patch -p1 -d $(PERCONA_SERVER)
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
