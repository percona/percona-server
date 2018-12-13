### Summary
Build is same as Percona Server 5.7, the details please refer to:   
https://www.percona.com/doc/percona-server/5.7/installation.html

### Compiling
```
git clone https://github.com/XeLabs/tokudb
cd  tokudb
git submodule init
git submodule update
```
The lazy way:
```
cd xelabs
bash build.sh
```
or 
```
mkdir build
cd build
cmake ..\
  -DCMAKE_BUILD_TYPE=RelWithDebInfo\
  -DBUILD_CONFIG=mysql_release\
  -DFEATURE_SET=community\
  -DWITH_EMBEDDED_SERVER=OFF\
  -DTOKUDB_VERSION=7.5.6\
  -DBUILD_TESTING=OFF\
  -DWITHOUT_ROCKSDB=ON\
  -DWITH_BOOST=../extra/boost/boost_1_59_0.tar.gz\
  -DCOMPILATION_COMMENT="XeLabs TokuDB build $(date +%Y%m%d.%H%M%S.$(git rev-parse --short HEAD))"\
  -DCMAKE_INSTALL_PREFIX=<your-install-dir>
make -j8
```

### Compiling for debugging

```
mkdir build
cd build
cmake ..\
  -DCMAKE_BUILD_TYPE=Debug\
  -DBUILD_CONFIG=mysql_release\
  -DFEATURE_SET=community\
  -DWITH_EMBEDDED_SERVER=OFF\
  -DTOKUDB_VERSION=7.5.6\
  -DBUILD_TESTING=OFF\
  -DWITHOUT_ROCKSDB=ON\
  -DWITH_BOOST=../extra/boost/boost_1_59_0.tar.gz\
  -DCMAKE_INSTALL_PREFIX=<your-install-dir>
```

### Installation
```
make install
```

### Install Database
```
$./bin/mysqld --defaults-file=[your-my-cnf] --initialize-insecure
$ mysql -uroot -h127.0.0.1
```
