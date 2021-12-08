#!/usr/bin/env bash
# By: Melroy van den Berg
# Description: Build Windows 64-bit release

rm -rf build_prod_win
mkdir build_prod_win
cd build_prod_win

x86_64-w64-mingw32.static-cmake -G Ninja .. &&
ninja

