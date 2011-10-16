FETCH_CMD=wget
MASTER_SITE=http://s3.amazonaws.com/percona.com/downloads/community
MYSQL_VERSION=5.1.59
PERCONA_SERVER_VERSION=rel13.0
PERCONA_SERVER         ?=Percona-Server-$(MYSQL_VERSION)-$(PERCONA_SERVER_VERSION)
PERCONA_SERVER_SHORT_1 ?=Percona-Server-$(MYSQL_VERSION)
PERCONA_SERVER_SHORT_2 ?=Percona-Server

all:  main install-lic misc handlersocket maatkit-udf autorun
	@echo ""
	@echo "Percona Server source code is ready"
	@echo "Now change directory to $(PERCONA_SERVER) define variables as show below"
	@echo ""
	export CFLAGS="-O2 -g -fmessage-length=0 -D_FORTIFY_SOURCE=2"
	export CXXFLAGS="-O2 -g -fmessage-length=0 -D_FORTIFY_SOURCE=2"
	export LIBS=-lrt
	@echo ""
	@echo "and run ./configure --with-plugins=partition,archive,blackhole,csv,example,federated,innodb_plugin --without-embedded-server --with-pic --with-extra-charsets=complex --with-ssl --enable-assembler --enable-local-infile --enable-thread-safe-client --enable-profiling --with-readline && make all install"
	@echo ""

autorun:
	cd $(PERCONA_SERVER) && ./BUILD/autorun.sh

handlersocket:
	cp -R HandlerSocket-Plugin-for-MySQL $(PERCONA_SERVER)/storage
	patch -p1 -d $(PERCONA_SERVER)/storage < handlersocket.patch

maatkit-udf:
	cp -R UDF "$(PERCONA_SERVER)"
	cd "$(PERCONA_SERVER)"/UDF && autoreconf --install

install-lic: 
	@echo "Installing license files"
	install -m 644 COPYING.* $(PERCONA_SERVER)

prepare: mysql-$(MYSQL_VERSION).tar.gz
	@echo "Prepare Percona Server sources"
	rm -rf mysql-$(MYSQL_VERSION)
	rm -rf $(PERCONA_SERVER)
	rm -rf $(PERCONA_SERVER_SHORT_1)
	rm -rf $(PERCONA_SERVER_SHORT_2)
	tar zxf mysql-$(MYSQL_VERSION).tar.gz
	mv mysql-$(MYSQL_VERSION) $(PERCONA_SERVER)
	ln -s $(PERCONA_SERVER) $(PERCONA_SERVER_SHORT_1)
	ln -s $(PERCONA_SERVER) $(PERCONA_SERVER_SHORT_2)
	ln -s ../patches $(PERCONA_SERVER)/patches
	ln -s ../quiltrc $(PERCONA_SERVER)/quiltrc
	(cd $(PERCONA_SERVER)/mysql-test && rm mtr mysql-test-run)

main: prepare
	(cd $(PERCONA_SERVER) && ../apply_patches)

regenerate: clean prepare
	(cd $(PERCONA_SERVER) && ../normalize_patches)

mysql-$(MYSQL_VERSION).tar.gz:
	@echo "Downloading MySQL sources from $(MASTER_SITE)"
	$(FETCH_CMD) $(MASTER_SITE)/mysql-$(MYSQL_VERSION).tar.gz


misc:
	@echo "Installing other files"
	install -m 644 lrusort.py $(PERCONA_SERVER)/scripts

clean:
	rm -rf mysql-$(MYSQL_VERSION) $(PERCONA_SERVER) $(PERCONA_SERVER_SHORT_1) $(PERCONA_SERVER_SHORT_2)
