# Windows miscialious files

The following additional files are required for packaging to Windows. Of-course together with the libreweb-browser.exe binary.

## PThread

* quserex.dll - The QueueUserAPCEx package, providing alertable (pre-emptive) asyncronous threads cancelation (**Source:** [gcc.gnu.org/pub/pthreads-win32](https://gcc.gnu.org/pub/pthreads-win32/QueueUserAPCEx/user/)). Which we most likely do **NOT** want to use iso libcurl. We should avoid using `pthread_cancel`.

## GTK

* gdbus.exe - Required for creating a gdbus daemon under Windows (**Source:** MXE cross-build)
* gspawn-win64-helper.exe - Required for starting processes under Windows (**Source:** MXE cross-build)
* gspawn-win64-console.exe - Required for starting processes under Windows (**Source:** MXE cross-build)

## GDB

* gdb.exe - For debugging the binary


