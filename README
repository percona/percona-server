- example to patch
    cd mysql-x.x.x
    (cd [thisdir]; cat `cat series`) | patch -p1

- example to build (x86_64 Linux GCC)
    export CFLAGS="-O2 -g -fmessage-length=0 -D_FORTIFY_SOURCE=2"
    export CXXFLAGS="-O2 -g -fmessage-length=0 -D_FORTIFY_SOURCE=2"
    export LIBS=-lrt
    ./configure ...(omitted)... \
          --without-plugin-innobase --with-plugin-innodb_plugin

  * The builtin InnoDB should be disabled.
    Because there are no file compatibility to XtraDB-10

