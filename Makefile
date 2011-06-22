MASTER_SITE:=http://www.percona.com/downloads/community
MYSQL_VERSION:=5.5.12
MYSQL_DIST:=mysql-$(MYSQL_VERSION).tar.gz
PLUGINS:=auth_pam.so test_auth_pam_client.so

all : $(PLUGINS)

$(MYSQL_DIST):
	wget $(MYSQL_DIST);

unpack: $(MYSQL_DIST)
	-rm -rf mysql-$(MYSQL_VERSION)
	tar zxf $(MYSQL_DIST)

clean:
	-rm $(PLUGINS) auth_pam.o lib_auth_pam_client.o test_auth_pam_client.o

.PHONY : unpack all clean

%.so : %.o
	ld -shared $^ -o $@ -lpam

auth_pam.so: auth_pam.o lib_auth_pam_client.o 
test_auth_pam_client.so: test_auth_pam_client.o lib_auth_pam_client.o

CC:=gcc
CPPFLAGS:=-isystem mysql-$(MYSQL_VERSION)/include
CFLAGS:=-fPIC -O3 -g -Wall -Wextra -Werror

auth_pam.o: auth_pam.c lib_auth_pam_client.h
lib_auth_pam_client.o: lib_auth_pam_client.c lib_auth_pam_client.h
test_auth_pam_client.o: test_auth_pam_client.c lib_auth_pam_client.h
