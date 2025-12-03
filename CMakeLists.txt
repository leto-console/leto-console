add_compile_definitions(${PRESET_DEFINE_MACRO})
add_compile_definitions(${PRESET_DEFINE_SCREEN})

set(COMMON_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/${PRESET_NAME}/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${COMMON_BIN_DIR})

file(GLOB CPP_COURCES 
	"Core/Src/*/*.cpp" "Core/Src/*/*.c" 
	"Core/Inc/*/*.cpp" "Core/Inc/*/*.c"
)

if (${PRESET_DEFINE_MACRO} STREQUAL "USE_HAL_DRIVER")

	cmake_minimum_required(VERSION 3.20)
	project("STM32_INS" C CXX ASM)

	file(GLOB HW_COURCES
		"Drivers/STM32F4xx_HAL_Driver/Src/*.c"
		"Core/Src/*.cpp" "Core/Src/*.c"
	)

	#add_executable(${PROJECT_NAME} "Core/Startup/startup_stm32f411xe.s" "${CPP_COURCES}")
	add_executable(${PROJECT_NAME}.elf "Core/Startup/startup_stm32f401ccux.s" "${CPP_COURCES}" "${HW_COURCES}")

	target_link_directories(${PROJECT_NAME}.elf PRIVATE "../STM32F4_Libraries/Lib/stm32-debug")
	target_link_libraries(${PROJECT_NAME}.elf PRIVATE STM32F4_Libraries)

	target_compile_definitions(${PROJECT_NAME}.elf PRIVATE
		USE_HAL_DRIVER
		STM32F411xE
	)

	# ------------------------------------------------------------------------------
	# Пути к заголовочным файлам (-I)
	# ------------------------------------------------------------------------------
	target_include_directories(${PROJECT_NAME}.elf PUBLIC
		Core/Inc
		Drivers/CMSIS/Device/ST/STM32F4xx/Include
		Drivers/CMSIS/Include
		Drivers/STM32F4xx_HAL_Driver/Inc
		Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
		../STM32F4_Libraries/Inc
	)

	# 5. Команда для автоматической генерации файлов прошивки (.bin и .hex) после сборки ELF
    add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.elf ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.elf ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.bin
        COMMAND ${CMAKE_SIZE} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${PROJECT_NAME}.elf
        COMMENT "Generating .hex, .bin and printing memory size..."
    )

else ()

	cmake_minimum_required(VERSION 3.10)
	project("STM32_INS")

	include_directories("Core/Inc")
	include_directories("../STM32F4_Libraries/Inc")
	include_directories("../STM32F4_Libraries/Src")

	if (${PRESET_DEFINE_MACRO} STREQUAL "__WIN__")
		if(MSVC)
			set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup") # /SUBSYSTEM:WINDOWS")
		endif()
		add_executable ("${PROJECT_NAME}" WIN32 "Core/Main/App.cpp" "${CPP_COURCES}")

		target_link_directories("${PROJECT_NAME}" PRIVATE "../STM32F4_Libraries/Lib/win-debug")
		target_link_libraries("${PROJECT_NAME}" PRIVATE STM32F4_Libraries)

	elseif (${PRESET_DEFINE_MACRO} STREQUAL "__LINUX__")

		find_package(Qt6 COMPONENTS Widgets REQUIRED Core)
		qt_standard_project_setup()
		add_executable ("${PROJECT_NAME}" "Core/Main/App.cpp" "${CPP_COURCES}")
		target_link_libraries("${PROJECT_NAME}" PRIVATE Qt6::Core Qt6::Widgets)
		
	elseif (${PRESET_DEFINE_MACRO} STREQUAL "__TERMUX__")

		file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../STM32F4_Libraries/Inc/GraphicsPC/web" 
			DESTINATION "${COMMON_BIN_DIR}")

		add_executable ("${PROJECT_NAME}" "Core/Main/App.cpp" "${CPP_COURCES}")
		target_link_libraries("${PROJECT_NAME}" PRIVATE STM32F4_Libraries)
		target_link_libraries("${PROJECT_NAME}" PRIVATE STM32F4_GamesCollection)

	endif ()
		
endif ()
