CC:=gcc
CPPFLAGS:=-isystem /usr/include/mysql
CFLAGS:=-fPIC -O3 -g -Wall -Wextra -Werror -fno-strict-aliasing

PLUGINS:=auth_pam.so test_auth_pam_client.so

all : $(PLUGINS)

clean:
	-rm $(PLUGINS) auth_pam.o lib_auth_pam_client.o test_auth_pam_client.o

install: $(PLUGINS)
	mysql_config --plugindir && install $(PLUGINS) $$(mysql_config --plugindir)

.PHONY : unpack all clean install

%.so : %.o
	ld -shared $^ -o $@ -lpam

auth_pam.so: auth_pam.o lib_auth_pam_client.o 
test_auth_pam_client.so: test_auth_pam_client.o lib_auth_pam_client.o


auth_pam.o: auth_pam.c lib_auth_pam_client.h
lib_auth_pam_client.o: lib_auth_pam_client.c lib_auth_pam_client.h
test_auth_pam_client.o: test_auth_pam_client.c lib_auth_pam_client.h
