#!/bin/sh
#
# Prepare minimal tarball with static version of V8 library and headers
# needed for its embedding.
#
# Based on build_v8.sh script from https://github.com/percona/mysql-shell-packaging/
# which is in its turn based on https://github.com/jeroen/build-v8-static/.
#
# Use next command for install dependencies on ubuntu:focal
# apt-get install -y file curl python3 xz-utils git pkg-config libglib2.0-dev lsb-release
# apt-get install -y apt-utils sudo
# apt-get install -y gcc-9 g++-9
# update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 90
# update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-9 90
#
# Also the next package is required for arm64 support
# apt-get install file
# apt-get install gcc-11-aarch64-linux-gnu g++-11-aarch64-linux-gnu
# update-alternatives --install /usr/bin/aarch64-linux-gnu-gcc aarch64-linux-gnu-gcc /usr/bin/aarch64-linux-gnu-gcc-11 60
# update-alternatives --install /usr/bin/aarch64-linux-gnu-g++ aarch64-linux-gnu-g++ /usr/bin/aarch64-linux-gnu-g++-11 60
#
# Next gn flags are required for arm support
# target_cpu="arm64" v8_target_cpu="arm64"
# v8_enable_turbofan=false
# v8_enable_webassembly=false

tag="12.9.202.22"
if [ "$1" ]
  then
    tag=$1;
fi
echo "Build v8. Version $tag"

GIT_SSL_NO_VERIFY=true git clone https://chromium.googlesource.com/chromium/tools/depot_tools
export PATH=$(pwd)/depot_tools:$PATH
yes | fetch --force v8
cd v8
gclient sync -D --force --reset
gclient sync -D --revision tags/$tag
#./build/install-build-deps.sh
gn gen "out.gn/static" -vv --fail-on-unused-args --args='v8_monolithic=true
            use_rtti=true
            v8_static_library=true
            v8_enable_sandbox=false
            v8_enable_pointer_compression=false
            is_clang=false
            is_asan=false
            is_debug=false
            is_official_build=false
            is_component_build=false
            treat_warnings_as_errors=false
            v8_enable_i18n_support=false
            v8_use_external_startup_data=false
            use_custom_libcxx=false
            v8_enable_maglev=false
            use_sysroot=false'

# Save some time by building libv8_monolith.a only and not things like d8 or
# unit tests.
ninja -C out.gn/static v8_monolith

# Prepare tarball with minimal set of v8 headers and library we just have
# built which can be used for embedding.
# We keep directory layout compatible with older full source and binaries
# tarballs.
#
# In theory we need to copy some generated headers as well. But we don't
# do this since we don't use include/v8-inspector-protocol.h, which relies
# on them.
mkdir -p out.gn/static_mini/v8/out.gn/static/obj/
cp out.gn/static/obj/libv8_monolith.a out.gn/static_mini/v8/out.gn/static/obj/
cp -r include out.gn/static_mini/v8/
# V8 source and binaries should come with v8 license copy.
cp LICENSE* out.gn/static_mini/v8/
cd out.gn/static_mini
tar -zcvf v8_$tag.tar.gz v8
cd ../../..
mv v8/out.gn/static_mini/v8_$tag.tar.gz .

echo "Done."
