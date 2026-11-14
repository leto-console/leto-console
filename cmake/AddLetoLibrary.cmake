
# Функция добавления импортированной библиотеки Leto
# Использование:
#   add_leto_library(LetoAPI)
#   add_leto_library(LetoCore COMPILE_DEFINITIONS <...>)
#   add_leto_library(LetoAPI LIB_DIR <...> BIN_DIR <...>)
#   add_leto_library(LetoCore LIB_DIR <...> BIN_DIR <...> COMPILE_DEFINITIONS <...>)
function(add_leto_library LIB_NAME)

    # Разбор именованных аргументов
    set(options "")
    set(oneValueArgs LIB_DIR BIN_DIR)
    set(multiValueArgs COMPILE_DEFINITIONS INTERFACE_LINK_LIBRARIES)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

	# Если не переданы, используем глобальные переменные
    if(NOT ARG_LIB_DIR)
        set(ARG_LIB_DIR "${LIB_DIR}")
    endif()
    if(NOT ARG_BIN_DIR)
        set(ARG_BIN_DIR "${BIN_DIR}")
    endif()

    # Платформо-зависимые настройки	
	if("${PRESET_DEFINE_MACRO}" STREQUAL "__STM32__")
        set(_type STATIC)
        set(_prefix "lib")
        set(_ext "a")
        set(_implib "")
        set(_location "${ARG_LIB_DIR}/${_prefix}${LIB_NAME}.${_ext}")
	elseif(WIN32)
        set(_type SHARED)
        set(_prefix "")
        set(_ext "lib")
        set(_implib "${ARG_LIB_DIR}/${LIB_NAME}.${_ext}")
        set(_location "${ARG_BIN_DIR}/${LIB_NAME}.dll")
    elseif(UNIX)
        set(_type STATIC)
        set(_prefix "lib")
        set(_ext "a")
        set(_implib "")
        set(_location "${ARG_LIB_DIR}/${_prefix}${LIB_NAME}.${_ext}")
	endif()

    # Создание импортированной цели
    add_library(${LIB_NAME} ${_type} IMPORTED GLOBAL)

    # Базовые свойства (общие для всех библиотек)
    set_target_properties(${LIB_NAME} PROPERTIES
        IMPORTED_LOCATION "${_location}"
        INTERFACE_INCLUDE_DIRECTORIES "$ENV{LETO_PATH}/${LIB_NAME}/include"
    )

    # IMPORTED_IMPLIB нужен только для Windows (SHARED)
    if(_implib)
        set_target_properties(${LIB_NAME} PROPERTIES
            IMPORTED_IMPLIB "${_implib}"
        )
    endif()

    # Дополнительные compile definitions (если переданы)
    if(ARG_COMPILE_DEFINITIONS)
        set_target_properties(${LIB_NAME} PROPERTIES
            INTERFACE_COMPILE_DEFINITIONS "${ARG_COMPILE_DEFINITIONS}"
        )
    endif()

	# транзитивные зависимости линковки
    if(ARG_INTERFACE_LINK_LIBRARIES)
        set_target_properties(${LIB_NAME} PROPERTIES
            INTERFACE_LINK_LIBRARIES "${ARG_INTERFACE_LINK_LIBRARIES}")
    endif()
	
endfunction()
