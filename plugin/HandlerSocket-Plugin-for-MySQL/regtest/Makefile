
SHELL=/bin/bash

all:
	if [ "`uname -o`" = "Cygwin" ]; then \
	  ../../scripts/vsenv90.bat -c 'make test'; \
	else \
	  $(MAKE) test; \
	fi

force:
	export CXX="$(CXX)"; \
	export SUNJDK="$(SUNJDK)"; \
	for i in test_*; do \
	  pushd $$i > /dev/null; \
	  echo -n "$$i ... "; \
	  if [ -f ./DONE ]; then \
	    echo "skip"; \
	  elif ! ./run.sh > test.log; then \
	    echo -n "FORCE ... "; \
	    $(MAKE) accept; \
	    if ! ./run.sh > test.log; then \
	      echo "FAILED"; \
	      exit 1; \
	    fi; \
	  else \
	    echo "success"; \
	  fi; \
	  touch ./DONE; \
	  popd > /dev/null; \
	done; \
	exit 0;

test:
	export CXX="$(CXX)"; \
	export SUNJDK="$(SUNJDK)"; \
	for i in test_*; do \
	  pushd $$i > /dev/null; \
	  echo -n "$$i ... "; \
	  if [ -f ./DONE ]; then \
	    echo "skip"; \
	  elif ! ./run.sh > test.log; then \
	    echo "FAILED"; \
	    exit 1; \
	  else \
	    echo "success"; \
	  fi; \
	  touch ./DONE; \
	  popd > /dev/null; \
	done; \
	exit 0;

retest: clean test

clean:
	for i in test_*; do \
	  pushd $$i > /dev/null; \
	  if [ -f Makefile ]; then \
	    $(MAKE) clean; \
	  fi; \
	  rm -rf ./GWS_db; \
	  rm -f ./DONE; \
	  popd > /dev/null; \
	done; \
	exit 0;
	rm -f test_*/*.log
	rm -f test_*/*.out
	rm -rf test_*/*_db
	rm -f test_*/*.dll
	rm -f test_*/*.idb
	rm -f test_*/*.obj
	rm -f test_*/*.manifest
	rm -f test_*/*.so

cleanprepare:
	rm -f ../run/*.exe
	rm -f ../run/poi.jar
	rm -f ../run/*.dll

