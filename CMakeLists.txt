add_compile_definitions(${PRESET_DEFINE_MACRO})
add_compile_definitions(${USE_SCREEN})

file(GLOB CPP_COURCES 
	"Core/Src/*/*.cpp" "Core/Src/*/*.c" 
	"Core/Inc/*/*.cpp" "Core/Inc/*/*.c"
)

cmake_minimum_required(VERSION 3.20)
project("leto-console" C CXX ASM)

if (${PRESET_DEFINE_MACRO} STREQUAL "__STM32__")

	file(GLOB HW_COURCES
		"Drivers/STM32F4xx_HAL_Driver/Src/*.c"
		"Core/Src/*.cpp" "Core/Src/*.c"
	)

	add_executable(${PROJECT_NAME} "Core/Startup/startup_stm32f401ccux.s" "${CPP_COURCES}" "${HW_COURCES}")

	set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".elf")

	target_compile_definitions(${PROJECT_NAME} PRIVATE
		USE_HAL_DRIVER
		${MCU_CHIP}
	)

	# ------------------------------------------------------------------------------
	# Настройки архитектуры из .cproject (STM32F401CCUx)
	# ------------------------------------------------------------------------------
	set(OBJECT_GEN_FLAGS "-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb")
	set(COMPILER_FLAGS "-ffunction-sections -fdata-sections -Wall -fstack-usage")
	# -fcyclomatic-complexity - Используется только в STM32CubeIDE

	# Флаги компиляции для C / C++ / ASM
	set(CMAKE_C_FLAGS   "${OBJECT_GEN_FLAGS} ${COMPILER_FLAGS}" CACHE INTERNAL "C Compiler Flags")
	set(CMAKE_CXX_FLAGS "${OBJECT_GEN_FLAGS} ${COMPILER_FLAGS} -fno-exceptions -fno-rtti -fno-use-cxa-atexit" CACHE INTERNAL "C++ Compiler Flags")
	set(CMAKE_ASM_FLAGS "${OBJECT_GEN_FLAGS} -x assembler-with-cpp" CACHE INTERNAL "ASM Compiler Flags")

	set(CMAKE_EXE_LINKER_FLAGS "-T${CMAKE_CURRENT_LIST_DIR}/${LINKER_SCRIPT} --specs=nano.specs -Wl,-Map=${CMAKE_PROJECT_NAME}.map -Wl,--gc-sections -mabi=aapcs -u _printf_float -Wl,--print-memory-usage" CACHE INTERNAL "Linker options")

	# ------------------------------------------------------------------------------
	# Пути к заголовочным файлам (-I)
	# ------------------------------------------------------------------------------
	target_include_directories(${PROJECT_NAME} PUBLIC
		Core/Inc
		Drivers/CMSIS/Device/ST/STM32F4xx/Include
		Drivers/CMSIS/Include
		Drivers/STM32F4xx_HAL_Driver/Inc
		Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
	)

	# 5. Команда для автоматической генерации файлов прошивки (.bin и .hex) после сборки ELF
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.elf ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.elf ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.bin
        COMMAND ${CMAKE_SIZE} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.elf
        COMMENT "Generating .hex, .bin and printing memory size..."
    )

else ()

	include_directories("Core/Inc")

	if (${PRESET_DEFINE_MACRO} STREQUAL "__WIN__")
		if(MSVC)
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup") # /SUBSYSTEM:WINDOWS")
		endif()
		add_executable ("${PROJECT_NAME}" WIN32 "Core/Main/App.cpp" "${CPP_COURCES}")

	elseif (${PRESET_DEFINE_MACRO} STREQUAL "__LINUX__")

		find_package(Qt6 COMPONENTS Widgets REQUIRED Core)
		qt_standard_project_setup()
		add_executable ("${PROJECT_NAME}" "Core/Main/App.cpp" "${CPP_COURCES}")
		target_link_libraries("${PROJECT_NAME}" PRIVATE Qt6::Core Qt6::Widgets)
		
	elseif (${PRESET_DEFINE_MACRO} STREQUAL "__TERMUX__")

		file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../LetoCore/include/GraphicsPC/web" 
			DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

		add_executable ("${PROJECT_NAME}" "Core/Main/App.cpp" "${CPP_COURCES}")

	endif ()
		
endif ()

if(LETO_PATH)
    message(STATUS "[${PROJECT_NAME}] Найдена переменная LETO_PATH. Подключаем готовые артефакты...")

	target_link_directories(${PROJECT_NAME} PUBLIC "${LETO_PATH}/LetoAPI/lib/${CORE_BUILD_PRESET}")
	target_link_directories(${PROJECT_NAME} PUBLIC "${LETO_PATH}/LetoCore/lib/${CORE_BUILD_PRESET}")

	target_include_directories(${PROJECT_NAME} PUBLIC "${LETO_PATH}/LetoAPI/include")
	target_include_directories(${PROJECT_NAME} PUBLIC "${LETO_PATH}/LetoCore/include")
else()
    message(STATUS "[${PROJECT_NAME}] Переменная LETO_PATH не найдена. Серверная сборка через FetchContent...")

	include(FetchContent)
    FetchContent_Declare(
        LetoCore
        GIT_REPOSITORY https://github.com/leto-console/LetoCore
        GIT_TAG        main
    )
    FetchContent_MakeAvailable(LetoCore)
endif()

target_link_libraries("${PROJECT_NAME}" PUBLIC LetoAPI)
target_link_libraries("${PROJECT_NAME}" PUBLIC LetoCore)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin     # Для исполняемых файлов
    ARCHIVE DESTINATION lib     # Для статических библиотек
    LIBRARY DESTINATION lib     # Для динамических библиотек
)
