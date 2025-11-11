#!/usr/bin/env bash
#
git clean -xffd
git reset --hard
#
git submodule foreach git clean -xffd
git submodule foreach git reset --hard
git submodule update --init --recursive
#
git pull
#
if [[ "$1" = "--build" ]]; then
  dpkg-buildpackage -b --no-sign
fi
