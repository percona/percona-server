#!/bin/sh

libtoolize --force --copy && aclocal && automake -c --foreign --add-missing && autoheader && autoconf

