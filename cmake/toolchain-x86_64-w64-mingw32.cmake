# Sample toolchain file for building for Windows from an Ubuntu Linux system.
#
# Typical usage:
#    *) install cross compiler: `sudo apt-get install mingw-w64 g++-mingw-w64`
#    *) mkdir build
#    *) cd build
#    *) cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/toolchains/x86_64-w64-mingw32.cmake ..

set(CMAKE_SYSTEM_NAME Windows)
set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)

if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    # Задаем жесткие пути к компиляторам из вашего MSYS2 UCRT64
    set(CMAKE_C_COMPILER "C:/msys64/ucrt64/bin/gcc.exe")
    set(CMAKE_CXX_COMPILER "C:/msys64/ucrt64/bin/g++.exe")

    # Жестко привязываем правильный линкер и архиватор из того же пакета
    set(CMAKE_LINKER "C:/msys64/ucrt64/bin/ld.exe" CACHE FILEPATH "Linker" FORCE)
    set(CMAKE_AR "C:/msys64/ucrt64/bin/gcc-ar.exe" CACHE FILEPATH "Archiver" FORCE)
    set(CMAKE_RANLIB "C:/msys64/ucrt64/bin/gcc-ranlib.exe")
else()
    # cross compilers to use for C and C++
    set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
    set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)
    SET(CMAKE_AR ${TOOLCHAIN_PREFIX}-ar)
    SET(CMAKE_RANLIB ${TOOLCHAIN_PREFIX}-ranlib)
endif()

# target environment on the build host system
# set 1st to dir with the cross compiler's C/C++ headers/libs
#set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX} /usr/${TOOLCHAIN_PREFIX}/sys-root/mingw)

# modify default behavior of FIND_XXX() commands to
# search for headers/libs in the target environment and
# search for programs in the build host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
