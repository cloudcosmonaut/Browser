#!/usr/bin/env bash
# By: Melroy van den Berg
# Description: Build Windows release

rm -rf build_prod_win
mkdir build_prod_win
cd build_prod_win
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/crosscompile_mingw32.cmake -G Ninja .. &&
ninja