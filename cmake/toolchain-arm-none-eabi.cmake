# ==============================================================================
# CMake Toolchain файл для STM32 (Cortex-M4F)
# ==============================================================================

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

# ------------------------------------------------------------------------------
# АВТОМАТИЧЕСКАЯ НАСТРОЙКА ПУТЕЙ ДЛЯ WINDOWS И DEBIAN
# ------------------------------------------------------------------------------
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(EXE_SUFFIX ".exe")
    set(TOOLCHAIN_PATH "D:/Programs/GNU Arm Embedded Toolchain/10 2021.10/bin") 
else()
    set(EXE_SUFFIX "")
    set(TOOLCHAIN_PATH "/usr/bin") 
endif()

# Явно задаем пути к бинарникам, чтобы CMake не искал их в системе
set(CMAKE_C_COMPILER   "${TOOLCHAIN_PATH}/arm-none-eabi-gcc${EXE_SUFFIX}")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PATH}/arm-none-eabi-g++${EXE_SUFFIX}")
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_PATH}/arm-none-eabi-gcc${EXE_SUFFIX}")
set(CMAKE_AR           "${TOOLCHAIN_PATH}/arm-none-eabi-gcc-ar${EXE_SUFFIX}")
set(CMAKE_OBJCOPY      "${TOOLCHAIN_PATH}/arm-none-eabi-objcopy${EXE_SUFFIX}" CACHE FILEPATH "Objcopy" FORCE)
set(CMAKE_OBJDUMP      "${TOOLCHAIN_PATH}/arm-none-eabi-objdump${EXE_SUFFIX}" CACHE FILEPATH "Objdump" FORCE)
set(CMAKE_SIZE         "${TOOLCHAIN_PATH}/arm-none-eabi-size${EXE_SUFFIX}"    CACHE FILEPATH "Size"    FORCE)
set(CMAKE_READELF      "${TOOLCHAIN_PATH}/arm-none-eabi-readelf${EXE_SUFFIX}" CACHE FILEPATH "Readelf" FORCE)

# Отключаем проверку компилятора
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# ------------------------------------------------------------------------------
# Конфигурация типов сборки (Debug / Release / MinSizeRel)
# ------------------------------------------------------------------------------
# Из .cproject: уровень отладки -g3, оптимизация для размера -Os
set(CMAKE_C_FLAGS_DEBUG "-g3 -Os" CACHE INTERNAL "C Debug Flags")
set(CMAKE_CXX_FLAGS_DEBUG "-g3 -Os" CACHE INTERNAL "C++ Debug Flags")

set(CMAKE_C_FLAGS_RELEASE "-Os -DNDEBUG" CACHE INTERNAL "C Release Flags")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -DNDEBUG" CACHE INTERNAL "C++ Release Flags")

# Кроссплатформенный поиск заголовочных файлов внутри тулчейна
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
