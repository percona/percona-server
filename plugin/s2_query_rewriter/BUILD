============================
BUILD with MySQL source code
============================
# cp -r PROJECT/s2_query_rewriter MYSQL_SRC/plugin/
# cp -r PROJECT/s2_udf            MYSQL_SRC/plugin/
# cp -r PROJECT/s2                MYSQL_SRC/plugin/s2_query_rewriter/
# ln -s MYSQL_SRC/plugin/s2_query_rewriter/s2 MYSQL_SRC/plugin/s2_udf/s2

## Build s2 library (libs2.so) manually (first)
# cd MYSQL_SRC/plugin/s2_query_rewriter/s2/
# sh ./build.sh

## make mysql server
# cd MYSQL_SRC
# mkdir build
# cd build
# cmake .. '-DBUILD_CONFIG=mysql_release' '-DCMAKE_INSTALL_PREFIX=/usr/local/mysql' '-DWITH_SSL=system' -DDOWNLOAD_BOOST=1 -DWITH_BOOST=../boost
# make -j5 install

## You need to copy MYSQL_SRC/plugin/s2_query_rewriter/s2/libs2.so to your system library directory (/usr/lib64/)
# cp MYSQL_SRC/plugin/s2_query_rewriter/s2/libs2.so /usr/lib64/