MYSQL_VERSION=5.1.67
PERCONA_SERVER_VERSION=rel14.4
PERCONA_SERVER         ?=Percona-Server-$(MYSQL_VERSION)-$(PERCONA_SERVER_VERSION)
PERCONA_SERVER_SHORT_1 ?=Percona-Server-$(MYSQL_VERSION)
PERCONA_SERVER_SHORT_2 ?=Percona-Server
KEWPIE ?=kewpie
BASEDIR = $(CURDIR)

all:  main install-lic handlersocket maatkit-udf autorun
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

prepare: 
	@echo "Prepare Percona Server sources"
	rm -rf $(PERCONA_SERVER) $(PERCONA_SERVER_SHORT_1)
	ln -s $(PERCONA_SERVER_SHORT_2) $(PERCONA_SERVER)
	ln -s $(PERCONA_SERVER_SHORT_2) $(PERCONA_SERVER_SHORT_1)

main: prepare

clean:
	rm -rf $(PERCONA_SERVER) $(PERCONA_SERVER_SHORT_1)

test-crashme:
	cd $(KEWPIE) && ./kewpie.py --suite=crashme --basedir=$(BASEDIR)/$(PERCONA_SERVER_SHORT_2)

test-sqlbench:
	cd $(KEWPIE) && ./kewpie.py --suite=sqlbench --basedir=$(BASEDIR)/$(PERCONA_SERVER_SHORT_2)

test-randgen:
	cd $(KEWPIE) && ./kewpie.py --suite=randgen_basic --basedir=$(BASEDIR)/$(PERCONA_SERVER_SHORT_2)

test-randgen-bugs:
	cd $(KEWPIE) && ./kewpie.py --suite=randgen_bugs --basedir=$(BASEDIR)/$(PERCONA_SERVER_SHORT_2)

test-cluster:
	cd $(KEWPIE) && ./kewpie.py --suite=cluster_basic,cluster_randgen --basedir=$(BASEDIR)/$(PERCONA_SERVER_SHORT_2)

test-cluster-basic:
	cd $(KEWPIE) && ./kewpie.py --suite=cluster_basic --basedir=$(BASEDIR)/$(PERCONA_SERVER_SHORT_2)

test-cluster-randgen:
	cd $(KEWPIE) && ./kewpie.py --suite=cluster_randgen --basedir=$(BASEDIR)/$(PERCONA_SERVER_SHORT_2)

test-cluster-bugs:
	cd $(KEWPIE) && ./kewpie.py --suite=cluster_bugs --basedir=$(BASEDIR)/$(PERCONA_SERVER_SHORT_2)

test-innodb-crash:
	cd $(KEWPIE) && ./kewpie.py --suite=innodbCrash --basedir=$(BASEDIR)/$(PERCONA_SERVER_SHORT_2)





