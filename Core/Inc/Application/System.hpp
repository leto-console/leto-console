/**
 * @file System.hpp
 * @date Dec 29, 2024
 * @author: Rakhimov T.
 */

#ifndef INC_APPLICATION_SYSTEM_HPP_
#define INC_APPLICATION_SYSTEM_HPP_

#ifdef __LINUX__
#	include <QApplication>
#endif

#ifndef USE_HAL_DRIVER
#	include <Utils/arg_parser.hpp>
#endif

#include <EEPROM/EEPROM_24C.hpp>

#if defined(USE_HAL_DRIVER)
#	if defined(USE_SSD1306)
#		include <SSD1306/SSD1306_HalDisplay.hpp>
#	elif defined(USE_ST7735)
#		include <ST7735/ST7735_HalDisplay.hpp>
#	endif
#elif defined(__WIN__)
#	include <GraphicsPC/WinDisplay.hpp>
#elif defined(__TERMUX__)
#	include <GraphicsPC/HttpDisplay.hpp>
#endif

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

#ifdef __LINUX__
	QApplication	*qapp;
#endif

	EEPROM_24C 		eeprom_24c_08;

#if defined (USE_HAL_DRIVER)
#	if defined(USE_SSD1306)
	SSD1306_HalDisplay display;
#	elif defined(USE_ST7735)
	ST7735_HalDisplay display;
#	endif
#elif defined(__WIN__)
	WinDisplay display;
#else defined (__TERMUX__)
	HttpDisplay display;
#endif

	StackGuard display_guard;

	// Массив указателей на устройства пользовательского ввода
	StaticList<UserInputDevice*, 32> UserInputs;

public:
	Application(int argc, char** argv);
	~Application();

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
#endif

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

#endif /* INC_APPLICATION_SYSTEM_HPP_ */
