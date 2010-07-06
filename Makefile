FETCH_CMD=wget
MASTER_SITE=http://www.percona.com/downloads/community
MYSQL_VERSION=5.1.48

all: main install-lic tests misc
	@echo ""
	@echo "Percona Server source code is ready"
	@echo "Now change directory to Percona-Server define variables as show below"
	@echo ""
	export CFLAGS="-O2 -g -fmessage-length=0 -D_FORTIFY_SOURCE=2"
	export CXXFLAGS="-O2 -g -fmessage-length=0 -D_FORTIFY_SOURCE=2"
	export LIBS=-lrt
	@echo ""
	@echo "and run ./configure ... --without-plugin-innobase --with-plugin-innodb_plugin && make all install"
	@echo ""


install-lic: 
	@echo "Installing license files"
	install -m 644 COPYING.* Percona-Server

main: mysql-$(MYSQL_VERSION).tar.gz
	@echo "Prepare Percona Server sources"
	rm -rf mysql-$(MYSQL_VERSION)
	rm -rf Percona-Server
	tar zxf mysql-$(MYSQL_VERSION).tar.gz
	mv mysql-$(MYSQL_VERSION) Percona-Server
	(cat `cat series`) | patch -p1 -d Percona-Server

mysql-$(MYSQL_VERSION).tar.gz:
	@echo "Downloading MySQL sources from $(MASTER_SITE)"
	$(FETCH_CMD) $(MASTER_SITE)/mysql-$(MYSQL_VERSION).tar.gz

tests:
	@echo "Installing mysql-test files"
	install -m 644 mysql-test/*.opt Percona-Server/mysql-test/t/
	install -m 644 mysql-test/*.test Percona-Server/mysql-test/t/
	install -m 644 mysql-test/*.result Percona-Server/mysql-test/r/
	install -m 644 mysql-test/*.inc Percona-Server/mysql-test/include/
misc:
	@echo "Installing other files"
	install -m 644 lrusort.py Percona-Server/scripts

clean:
	rm -rf mysql-$(MYSQL_VERSION) Percona-Server
	rm -f mysql-$(MYSQL_VERSION).tar.gz

