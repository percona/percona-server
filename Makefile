
compile:
	cd handlersocket && $(MAKE)
	cd perl-Net-HandlerSocket && perl Makefile.PL && $(MAKE)

rpms: rpm_cli rpm_perl rpm_c

rpm_cli: clean_cli
	tar cvfz dist/libhsclient.tar.gz libhsclient
	rpmbuild --define "_topdir `pwd`/dist" -ta \
		dist/libhsclient.tar.gz

rpm_perl: clean_perl
	cd perl-Net-HandlerSocket && perl Makefile.PL && $(MAKE) clean && \
		rm -f Makefile.old
	tar cvfz dist/perl-Net-HandlerSocket.tar.gz perl-Net-HandlerSocket
	rpmbuild --define "_topdir `pwd`/dist" -ta \
		dist/perl-Net-HandlerSocket.tar.gz

rpm_c: clean_c
	tar cvfz dist/handlersocket.tar.gz handlersocket
	rpmbuild --define "_topdir `pwd`/dist" -ta \
		dist/handlersocket.tar.gz

install_rpm_pl:
	- sudo rpm -e perl-Net-HandlerSocket
	- sudo rpm -e perl-Net-HandlerSocket-debuginfo
	$(MAKE) clean
	$(MAKE) rpm_perl
	- sudo rpm -U dist/RPMS/*/*.rpm

installrpms:
	- sudo rpm -e handlersocket
	- sudo rpm -e handlersocket-debuginfo
	- sudo rpm -e perl-Net-HandlerSocket
	- sudo rpm -e perl-Net-HandlerSocket-debuginfo
	- sudo rpm -e libhsclient
	- sudo rpm -e libhsclient-debuginfo
	$(MAKE) clean
	$(MAKE) rpm_cli
	- sudo rpm -U dist/RPMS/*/*.rpm
	$(MAKE) clean
	$(MAKE) rpm_perl
	- sudo rpm -U dist/RPMS/*/*.rpm
	$(MAKE) clean
	$(MAKE) rpm_c
	- sudo rpm -U dist/RPMS/*/*.rpm

clean_cli:
	cd libhsclient && $(MAKE) clean
	cd client && $(MAKE) clean

clean_perl:
	cd perl-Net-HandlerSocket && perl Makefile.PL && $(MAKE) clean && \
		rm -f Makefile.old

clean_c:
	cd handlersocket && $(MAKE) clean

clean: clean_cli clean_perl clean_c
	cd regtest && $(MAKE) clean
	rm -rf dist/*/*
	rm -f dist/*.tar.gz

