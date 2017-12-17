set x

cmake ..\
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DBUILD_CONFIG=mysql_release \
  -DFEATURE_SET=community \
  -DWITH_EMBEDDED_SERVER=OFF \
  -DTOKUDB_VERSION=7.5.6 \
  -DBUILD_TESTING=OFF \
  -DWITHOUT_ROCKSDB=ON \
  -DWITH_BOOST=../extra/boost/boost_1_59_0.tar.gz \
  -DCOMPILATION_COMMENT="XeLabs TokuDB build $(date +%Y%m%d.%H%M%S.$(git rev-parse --short HEAD))" \
  -DCMAKE_INSTALL_PREFIX="/usr"
make -j8
