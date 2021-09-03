# Cross-compile on Linux to a native Windows binary
# Install?: https://www.gtk.org/docs/installations/windows/
#
# Build usage: cmake -DCMAKE_TOOLCHAIN_FILE=cmake/crosscompile_mingw32.cmake

set(CMAKE_SYSTEM_NAME Windows)

# Newer mingw packages available via PPA packport: https://launchpad.net/~noasakurajin/+archive/ubuntu/sid-ubuntu-ports
set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)
# Win64 toolchain bundle location
# Using bundle: https://gitlab.melroy.org/melroy/gtk-3-bundle-for-windows/-/tree/main
set(WINDOWS_TOOLCHAIN_PATH /home/melroy/Documents/gtk3_bundle_3.24.30_win64)

# which compilers to use for C and C++
set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)

# Where is the target environment
set(CMAKE_FIND_ROOT_PATH
  /usr/${TOOLCHAIN_PREFIX}
  ${WINDOWS_TOOLCHAIN_PATH}
)

# Add the includes directory of the Windows target system
foreach(PATH IN LISTS CMAKE_FIND_ROOT_PATH)
  include_directories(BEFORE ${PATH}/include)
endforeach()

# Add includes manually for win64, why doesn't Cmake just recognize those include paths..?
include_directories(BEFORE 
  ${WINDOWS_TOOLCHAIN_PATH}/include/gtkmm-3.0
  ${WINDOWS_TOOLCHAIN_PATH}/include/gdkmm-3.0
  ${WINDOWS_TOOLCHAIN_PATH}/lib/gdkmm-3.0/include
  ${WINDOWS_TOOLCHAIN_PATH}/include/pangomm-1.4
  ${WINDOWS_TOOLCHAIN_PATH}/lib/pangomm-1.4/include
  ${WINDOWS_TOOLCHAIN_PATH}/include/cairomm-1.0
  ${WINDOWS_TOOLCHAIN_PATH}/lib/cairomm-1.0/include
  ${WINDOWS_TOOLCHAIN_PATH}/include/glibmm-2.4
  ${WINDOWS_TOOLCHAIN_PATH}/lib/glibmm-2.4/include
  ${WINDOWS_TOOLCHAIN_PATH}/include/atkmm-1.6
  ${WINDOWS_TOOLCHAIN_PATH}/lib/atkmm-1.6/include
  ${WINDOWS_TOOLCHAIN_PATH}/include/sigc++-2.0
  ${WINDOWS_TOOLCHAIN_PATH}/lib/sigc++-2.0/include
  ${WINDOWS_TOOLCHAIN_PATH}/include/giomm-2.4
  ${WINDOWS_TOOLCHAIN_PATH}/lib/giomm-2.4/include
  ${WINDOWS_TOOLCHAIN_PATH}/include/gtk-3.0
  ${WINDOWS_TOOLCHAIN_PATH}/include/gdk-pixbuf-2.0
  ${WINDOWS_TOOLCHAIN_PATH}/include/glib-2.0
  ${WINDOWS_TOOLCHAIN_PATH}/lib/glib-2.0/include
  ${WINDOWS_TOOLCHAIN_PATH}/include/gio-win32-2.0
)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -Os")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -Os")
