#!/bin/bash

url="http://www.percona.com/redir/downloads/Percona-Server-5.5/Percona-Server-5.5.20-24.1/binary/linux/`uname -m`"
tarball="Percona-Server-5.5.20-rel24.1-217.Linux.`uname -m`.tar.gz"
destdir="./server"

if test -d "$destdir"
then
    rm -rf "$destdir"
fi
mkdir "$destdir"
    

wget -q $url/$tarball
tar xzf $tarball -C ./server 
