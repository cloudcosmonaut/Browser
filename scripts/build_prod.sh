#!/usr/bin/env bash
# By: Melroy van den Berg
# Description: Linux release (production) build + create Debian package file (.deb), 
#  RPM [Red Hat] Package Manager (.rpm) and compressed file (.tgz/.tar.gz)
#
#  Installs into /usr folder (prefix)

rm -rf build_prod
mkdir build_prod
cd build_prod
cmake -G Ninja -DCMAKE_INSTALL_PREFIX:PATH=/usr -DCMAKE_BUILD_TYPE=Release ..
ninja && 
echo "INFO: Building packages...";
cpack -G "TGZ;DEB;RPM"
