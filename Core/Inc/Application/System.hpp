/**
 * @file System.hpp
 * @date Dec 29, 2024
 * @author: Rakhimov T.
 */

#ifndef INC_APPLICATION_SYSTEM_HPP_
#define INC_APPLICATION_SYSTEM_HPP_

#ifndef USE_HAL_DRIVER
#	include <Utils/arg_parser.hpp>
#endif

#if defined(USE_HAL_DRIVER)
#	if defined(USE_SSD1306)
#		include <SSD1306/SSD1306_HalDisplay.hpp>
		using ApplicationDisplay = SSD1306_HalDisplay;
#	elif defined(USE_ST7735)
#		include <ST7735/ST7735_HalDisplay.hpp>
		using ApplicationDisplay = ST7735_HalDisplay;
#	endif
#elif defined(__WIN__)
#	include <GraphicsPC/WinDisplay.hpp>
	using ApplicationDisplay = WinDisplay;
#elif defined(__TERMUX__)
#	include <GraphicsPC/HttpDisplay.hpp>
	using ApplicationDisplay = HttpDisplay;
#elif defined(__LINUX__)
#	include <GraphicsPC/SDLDisplay.hpp>
	using ApplicationDisplay = SDLDisplay;
#endif

#include <EEPROM/EEPROM_24C.hpp>
#include <Input/Devices/UserInputDevice.hpp>
#include <Storage/Storage.hpp>
#include <System/SystemMode.hpp>
#include <Data/StaticList.hpp>
#include <System/StackGuard.hpp>

class Application
{
protected:
#ifdef USE_HAL_DRIVER
	I2C_HandleTypeDef* hi2c				= nullptr;
	TIM_HandleTypeDef* htim				= nullptr;
	UART_HandleTypeDef* huart_console	= nullptr;
	UART_HandleTypeDef* huart_web		= nullptr;
	RTC_HandleTypeDef* hrtc 			= nullptr;
#else
	ArgParser arg_parser;
#endif

	EEPROM_24C eeprom_24c_08;

	ApplicationDisplay display;

	StackGuard display_guard;

public:
#ifdef USE_HAL_DRIVER
	struct Periphery
	{
		I2C_HandleTypeDef* 	hi2c 			= nullptr;
		TIM_HandleTypeDef* 	htim 			= nullptr;
		UART_HandleTypeDef* huart_console	= nullptr;
		UART_HandleTypeDef* huart_web		= nullptr;
		RTC_HandleTypeDef*	hrtc			= nullptr;
		};

	Application(Periphery periphery);
#else
	Application(int argc, char** argv);
#endif
	~Application();

	// Массив указателей на устройства пользовательского ввода
	StaticList<UserInputDevice*, 32> UserInputs;

	void Init();
	bool Loop();

	bool GetRebootFlag() const;

protected:
	/* Флаг перезагрузки. Выставляется перед выходом из цикла */
	bool rebootFlag = false;

	// Инициализация режима работы системы
	void InitOnSystemModeChanged();

	/* Блок с функциями инициализации */

	// Иинициализация системного ввода
	void InitInput();

	// Инициализация внешних устройств
	void InitExtDev();

	/* SYSTEM */

	// Инициализация системных сцен
	void InitSystemScenes();

	// Иинициализация системных задач
	void InitSystemTasks();

	/* USER */

	// Инициализация Пользовательских сцен
	void InitUserScenes();

	// Иинициализация пользовательских задач
	void InitUserTasks();
};

extern Application* SysInstance;

#endif /* INC_APPLICATION_SYSTEM_HPP_ */
