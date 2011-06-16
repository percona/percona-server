WGET:=wget
MASTER_SITE:=http://www.percona.com/downloads/community
MYSQL_VERSION:=5.5.12
MYSQL_DIST:=mysql-$(MYSQL_VERSION).tar.gz

all : auth_pam.so

$(MYSQL_DIST):
	$(WGET) $(MYSQL_DIST);

unpack: $(MYSQL_DIST)
	-rm -rf mysql-$(MYSQL_VERSION)
	tar zxf $(MYSQL_DIST)

clean:
	-rm auth_pam.so auth_pam.o

.PHONY : unpack all clean

auth_pam.so: auth_pam.o
	ld -shared $< -o $@ -lpam

CC:=gcc
CPPFLAGS:=-isystem mysql-$(MYSQL_VERSION)/include
CFLAGS:=-fPIC -O3 -g -Wall -Wextra # -Werror

auth_pam.o: auth_pam.c

.DEFAULT : auth_pam.so
