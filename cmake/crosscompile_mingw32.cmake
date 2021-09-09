# Cross-compile on Linux to a native Windows binary
#
# Build usage: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/crosscompile_mingw32.cmake

set(CMAKE_SYSTEM_NAME Windows)

# Newer mingw packages available via PPA packport: https://launchpad.net/~noasakurajin/+archive/ubuntu/sid-ubuntu-ports
set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)
# Win64 toolchain bundle location
# Using bundle: https://gitlab.melroy.org/melroy/gtk-3-bundle-for-windows/-/tree/main
set(WINDOWS_TOOLCHAIN_PATH /home/melroy/Documents/gtk3_bundle_3.24.30_win64)

set(CMAKE_SYSROOT ${WINDOWS_TOOLCHAIN_PATH})

# which compilers to use for C and C++
# Switch to Posix compiles to get std::thread working (insto win32)
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc-posix)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++-posix)
set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)

# Ensure that FIND_PACKAGE() functions and friends look in the rootfs for the target environment
set(CMAKE_FIND_ROOT_PATH
  /usr/${TOOLCHAIN_PREFIX}
  ${WINDOWS_TOOLCHAIN_PATH}
)

# Add includes manually for win64, why doesn't Cmake just recognize those include paths..?
# /usr/bin/x86_64-w64-mingw32-pkg-config --cflags gtkmm-3.0
#  Looks like the cflags (which include -I) are not correctly returned by pkgconfig (it's resolves to /mingw64 folder)
set(SYSTEM_INCLUDES 
  /usr/${TOOLCHAIN_PREFIX}/include
  ${WINDOWS_TOOLCHAIN_PATH}/include
  ${WINDOWS_TOOLCHAIN_PATH}/include/atk-1.0
  ${WINDOWS_TOOLCHAIN_PATH}/include/gtkmm-3.0
  ${WINDOWS_TOOLCHAIN_PATH}/lib/gtkmm-3.0/include
  ${WINDOWS_TOOLCHAIN_PATH}/include/harfbuzz
  ${WINDOWS_TOOLCHAIN_PATH}/include/gdkmm-3.0
  ${WINDOWS_TOOLCHAIN_PATH}/lib/gdkmm-3.0/include
  ${WINDOWS_TOOLCHAIN_PATH}/include/pango-1.0
  ${WINDOWS_TOOLCHAIN_PATH}/include/pangomm-1.4
  ${WINDOWS_TOOLCHAIN_PATH}/lib/pangomm-1.4/include
  ${WINDOWS_TOOLCHAIN_PATH}/include/cairo
  ${WINDOWS_TOOLCHAIN_PATH}/include/cairomm-1.0
  ${WINDOWS_TOOLCHAIN_PATH}/lib/cairomm-1.0/include
  ${WINDOWS_TOOLCHAIN_PATH}/include/glibmm-2.4
  ${WINDOWS_TOOLCHAIN_PATH}/lib/glibmm-2.4/include
  ${WINDOWS_TOOLCHAIN_PATH}/include/atkmm-1.6
  ${WINDOWS_TOOLCHAIN_PATH}/lib/atkmm-1.6/include
  ${WINDOWS_TOOLCHAIN_PATH}/include/sigc++-2.0
  ${WINDOWS_TOOLCHAIN_PATH}/lib/sigc++-2.0/include
  ${WINDOWS_TOOLCHAIN_PATH}/include/freetype2
  ${WINDOWS_TOOLCHAIN_PATH}/include/giomm-2.4
  ${WINDOWS_TOOLCHAIN_PATH}/lib/giomm-2.4/include
  ${WINDOWS_TOOLCHAIN_PATH}/include/gtk-3.0
  ${WINDOWS_TOOLCHAIN_PATH}/include/gdk-pixbuf-2.0
  ${WINDOWS_TOOLCHAIN_PATH}/include/glib-2.0
  ${WINDOWS_TOOLCHAIN_PATH}/lib/glib-2.0/include
  ${WINDOWS_TOOLCHAIN_PATH}/include/gio-win32-2.0
)

include_directories(SYSTEM ${SYSTEM_INCLUDES})

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Set the mingw pkg-config executable
set(PKG_CONFIG_EXECUTABLE /usr/bin/${TOOLCHAIN_PREFIX}-pkg-config CACHE PATH "pkg-config executable")
# Change the environment variables to find the target pkgconfig files
set(ENV{PKG_CONFIG_PATH} ${WINDOWS_TOOLCHAIN_PATH}/share/pkgconfig)
set(ENV{PKG_CONFIG_LIBDIR} /usr/${TOOLCHAIN_PREFIX}/lib/pkgconfig:${WINDOWS_TOOLCHAIN_PATH}/lib/pkgconfig)
SET(ENV{PKG_CONFIG_SYSROOT_DIR} /usr/${TOOLCHAIN_PREFIX}:${WINDOWS_TOOLCHAIN_PATH})

# Most likely .. mingw32-ld is also searching in /mingw64 by default for not only the includes, but also the libs?

# These variables make sure that pkg-config does never discard standard
# include and library paths from the compile and linking flags.
SET(ENV{PKG_CONFIG_ALLOW_SYSTEM_CFLAGS} 1)
SET(ENV{PKG_CONFIG_ALLOW_SYSTEM_LIBS} 1)
SET(PKG_CONFIG_USE_CMAKE_PREFIX_PATH TRUE)


# Supprese warnings via -Wno-attributes (see also: https://github.com/Beep6581/RawTherapee/issues/6105)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -Os -Wno-attributes")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -Os -Wno-attributes")