/**
 * @file System.cpp
 * @date Dec 29, 2024
 * @author: Rakhimov T.
 */

#include "Application/System.hpp"

#include <LetoAPI_V1_System/Make.hpp>

#include <SceneManager/LambdaScene.hpp>
#include <SceneManager/SceneManager.hpp>

#include <Time/TimeUtils.hpp>

#include <cinttypes>
#include <Data/StaticText.hpp>

#ifdef _WIN32
#include <string>
#endif

// ----------------------------------------------------------------------------------------------------
//  Платформозависимые включения и инициализации приложения (HAL, Win32, Linux)
// ----------------------------------------------------------------------------------------------------

#ifdef USE_HAL_DRIVER

#include <HAL_include/HAL.hpp>
#include "main.h"

#else

#include <iostream>
#include <Utils/FileConfig.hpp>

#endif

// ------------------------------------- Инициализация HAL-версии -------------------------------------

#ifdef USE_HAL_DRIVER

Application::Application(Periphery periphery) :
	hi2c{ periphery.hi2c },
	htim{ periphery.htim },
	huart_console{ periphery.huart_console },
	huart_web{ periphery.huart_web },
	hrtc{ periphery.hrtc },
	eeprom_24c_08{ hi2c, (256 / 8) * 1024 },
#if defined(USE_SSD1306)
	display{ hi2c, true }
#elif defined(USE_ST7735)
	display{ nullptr }
#endif
{
}

Application::~Application()
{
}
#endif

// ----------------------------------- Инициализация Windows-версии -----------------------------------

#ifdef __WIN__
#include <windows.h>

Application::Application(int argc, char** argv) :
	arg_parser{ argc, argv },
	eeprom_24c_08{ (256 / 8) * 1024, arg_parser.FindArg("--client") ? "client.eeprom" : "server.eeprom" },
#if defined(USE_SSD1306)
	display{ 128, 64, 3 }
#else if defined(USE_ST7735)
	display{ 160, 128, 3 }
#endif
{
}
Application::~Application()
{
}
#endif

// ------------------------------------- Инициализация Linux-версии -------------------------------------

#ifdef __LINUX__
#include <SSD1306/SSD1306_QtWindow.hpp>
#include <QThread>

Application::Application()
{
	int _ = 0; qapp = new QApplication(_, nullptr);
	ssd1306_w = new SSD1306_QtWindow();
}
Application::~Application()
{
	delete ssd1306_w;
	delete qapp;
}
#endif

// ------------------------------------- Инициализация HTTP-версии -------------------------------------

#ifdef __TERMUX__
#include <GraphicsPC/HttpDisplay.hpp>

Application::Application(int argc, char** argv) :
	arg_parser{ argc, argv },
	eeprom_24c_08{ (256 / 8) * 1024, arg_parser.FindArg("--client") ? "client.eeprom" : "server.eeprom" },
#if defined(USE_SSD1306)
	display{ 128, 64, 3 }
#else if defined(USE_ST7735)
	display{ 160, 128, 3 }
#endif
{
}
Application::~Application()
{
}
#endif


// ----------------------------------------------------------------------------------------------------

#include <CommandHandler/CommandHandler.hpp>
#include <CommandHandler/ConsoleCommand.hpp>
#include <TaskHandler/PriorityTaskSheduler.hpp>
#include <Graphics/IDisplayTasks.hpp>
#include <Input/UserInputTask.hpp>
#include <GamesSupport/GameCenter.hpp>

class PrintCommand : public ConsoleCommand
{
public:
	PrintCommand(const StaticText32& name, const StaticText32& descr)
		: ConsoleCommand{ name, descr }
	{
	}
	void Handle(const StaticListView<StaticText32>& args) override
	{
		if (args.size() == 1)
		{
			for (PriorityTask* task : PriorityTaskSheduler::GetList())
			{
				printf("[Task] %s - %f\n", task->GetName().ConstChar(), task->GetAverageDurationMs());
			}
			return;
		}
		else if (args.size() == 2)
		{
			printf("%s\n", args[1].ConstChar());
			return;
		}
		printf("Invalid args\n");
	}
};

// ----------------------------------------------------------------------------------------------------

#ifdef USE_HAL_DRIVER

#define GPIO_PIN_SCL GPIO_PIN_8
#define GPIO_PIN_SDA GPIO_PIN_9

void I2C_ResetBus(I2C_HandleTypeDef* hi2c)
{
	// Отключаем I2C
	HAL_I2C_DeInit(hi2c);

	// Переводим пины в GPIO режим
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };
	GPIO_InitStruct.Pin = GPIO_PIN_SDA | GPIO_PIN_SCL; // укажите ваши пины
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); // например, порт B

	// Генерируем 9+ импульсов CLK, чтобы "сбросить" slave
	for (int i = 0; i < 10; i++) {
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_SCL, GPIO_PIN_RESET);
		HAL_Delay(1); // !!!!! TODO: изменен SysTick
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_SCL, GPIO_PIN_SET);
		HAL_Delay(1); // !!!!! TODO: изменен SysTick
	}

	// Формируем STOP: SDA = 0 -> 1 при SCL = 1
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_SDA, GPIO_PIN_RESET);
	HAL_Delay(1); // !!!!! TODO: изменен SysTick
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_SCL, GPIO_PIN_SET);
	HAL_Delay(1); // !!!!! TODO: изменен SysTick
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_SDA, GPIO_PIN_SET);
	HAL_Delay(1); // !!!!! TODO: изменен SysTick

	// Возвращаем пины в I2C режим
	GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	// Перезапускаем I2C
	HAL_I2C_Init(hi2c);
}

class I2CCom : public ConsoleCommand
{
protected:
	I2C_HandleTypeDef* hi2c;
public:
	I2CCom(const StaticText32& my_name, const StaticText32& descr, I2C_HandleTypeDef* hi2c)
		: ConsoleCommand{ my_name, descr }, hi2c{ hi2c }
	{
	}
	void Handle(const StaticListView<StaticText32>& args) override
	{
		if (args.size() == 2)
		{
			if (args[1] == "restart")
			{
				I2C_ResetBus(hi2c);
				return;
			}
		}
		printf("Invalid args\n");
	}
};

#include <NRF24L01/nrf24l01.h>

void PrintNRFInfo(UART_HandleTypeDef* huart)
{
  char str[64] = {0,};

	uint8_t status = get_status();
	  snprintf(str, 64, "get_status: 0x%02x\n\r", status);
	  HAL_UART_Transmit(huart, (uint8_t*)str, strlen(str), 1000);

	  status = getPALevel();
	  snprintf(str, 64, "getPALevel: 0x%02x  ", status);
	  HAL_UART_Transmit(huart, (uint8_t*)str, strlen(str), 1000);

	  if(status == 0x00)
	  {
		  HAL_UART_Transmit(huart, (uint8_t*)"RF24_PA_MIN\n\r", strlen("RF24_PA_MIN\n\r"), 1000);
	  }
	  else if(status == 0x01)
	  {
		  HAL_UART_Transmit(huart, (uint8_t*)"RF24_PA_LOW\n\r", strlen("RF24_PA_LOW\n\r"), 1000);
	  }
	  else if(status == 0x02)
	  {
		  HAL_UART_Transmit(huart, (uint8_t*)"RF24_PA_HIGH\n\r", strlen("RF24_PA_HIGH\n\r"), 1000);
	  }
	  else if(status == 0x03)
	  {
		  HAL_UART_Transmit(huart, (uint8_t*)"RF24_PA_MAX\n\r", strlen("RF24_PA_MAX\n\r"), 1000);
	  }

	  status = getChannel();
	  snprintf(str, 64, "getChannel: 0x%02x № %d\n\r", status, status);
	  HAL_UART_Transmit(huart, (uint8_t*)str, strlen(str), 1000);

	  status = getDataRate();
	  snprintf(str, 64, "getDataRate: 0x%02x  ", status);
	  HAL_UART_Transmit(huart, (uint8_t*)str, strlen(str), 1000);

	  if(status == 0x02)
	  {
		  HAL_UART_Transmit(huart, (uint8_t*)"RF24_250KBPS\n\r", strlen("RF24_250KBPS\n\r"), 1000);
	  }
	  else if(status == 0x01)
	  {
		  HAL_UART_Transmit(huart, (uint8_t*)"RF24_2MBPS\n\r", strlen("RF24_2MBPS\n\r"), 1000);
	  }
	  else
	  {
		  HAL_UART_Transmit(huart, (uint8_t*)"RF24_1MBPS\n\r", strlen("RF24_1MBPS\n\r"), 1000);
	  }

	  status = getPayloadSize();
	  snprintf(str, 64, "getPayloadSize: %d\n\r", status);
	  HAL_UART_Transmit(huart, (uint8_t*)str, strlen(str), 1000);

	  status = getCRCLength();
	  snprintf(str, 64, "getCRCLength: 0x%02x  ", status);
	  HAL_UART_Transmit(huart, (uint8_t*)str, strlen(str), 1000);

	  if(status == 0x00)
	  {
		  HAL_UART_Transmit(huart, (uint8_t*)"RF24_CRC_DISABLED\n\r", strlen("RF24_CRC_DISABLED\n\r"), 1000);
	  }
	  else if(status == 0x01)
	  {
		  HAL_UART_Transmit(huart, (uint8_t*)"RF24_CRC_8\n\r", strlen("RF24_CRC_8\n\r"), 1000);
	  }
	  else if(status == 0x02)
	  {
		  HAL_UART_Transmit(huart, (uint8_t*)"RF24_CRC_16\n\r", strlen("RF24_CRC_16\n\r"), 1000);
	  }
}

void PrintNRFExtra(UART_HandleTypeDef* huart)
{
	  //HAL_Delay(1000);
	  uint8_t dt_reg = 0;
	  char str1[60]{};

	  dt_reg = read_register(NRF_CONFIG);

	  //sprintf(str1,"CONFIG: %02X \r\n",dt_reg);
	  //HAL_UART_Transmit(huart,(uint8_t*)str1,strlen(str1),0x1000);

	  printf("CONFIG: %02X \r\n", dt_reg);

	  dt_reg = 0;
	  dt_reg = read_register(EN_AA);

	  printf("EN_AA: %02X \r\n", dt_reg);

	  //sprintf(str1,"EN_AA: %02X \r\n",dt_reg);
	  //HAL_UART_Transmit(huart,(uint8_t*)str1,strlen(str1),0x1000);

	  dt_reg = 0;
	  dt_reg = read_register(EN_RXADDR);

	  sprintf(str1,"EN_RXADDR: 0x%02X\r\n",dt_reg);

	  HAL_UART_Transmit(huart,(uint8_t*)str1,strlen(str1),0x1000);

	  dt_reg = 0;
	  dt_reg = read_register(NRF_STATUS);

	  sprintf(str1,"STATUS: 0x%02X\r\n",dt_reg);

	  HAL_UART_Transmit(huart,(uint8_t*)str1,strlen(str1),0x1000);

	  dt_reg = 0;
	  dt_reg = read_register(RF_SETUP);

	  sprintf(str1,"RF_SETUP: 0x%02X\r\n",dt_reg);



}

class NRFCom : public ConsoleCommand
{
protected:
	UART_HandleTypeDef* huart;
public:
	NRFCom(const StaticText32& my_name, const StaticText32& descr, UART_HandleTypeDef* huart)
		: ConsoleCommand{ my_name, descr }, huart{ huart }
	{
	}
	void Handle(const StaticListView<StaticText32>& args) override
	{
		if (args.size() == 2)
		{
			if (args[1] == "info")
			{
				PrintNRFInfo(huart);
				return;
			}
			else if (args[1] == "extra")
			{
				PrintNRFExtra(huart);
				return;
			}
		}
		printf("Invalid args\n");
	}
};


class AllocCom : public ConsoleCommand
{
public:
	AllocCom(const StaticText32& my_name, const StaticText32& descr)
		: ConsoleCommand{ my_name, descr }
	{
	}
	void Handle(const StaticListView<StaticText32>& args) override
	{
		printf("CommonAllocator: %f\n", CommonAllocator.GetPercentage());
		printf("SystemAllocator: %f\n", SystemAllocator.GetPercentage());
		//printf("API_Allocator: %f\n", .GetPercentage());
	}
};

#endif

// ----------------------------------------------------------------------------------------------------

#include "Web/WebQueueRxTxTask.hpp"
#include "ProjectSettings/ProjectSettings.hpp"

#if defined (USE_HAL_DRIVER)
#include <UART/UART_console_init.hpp>
#endif

#include <Auth/AuthHandler.hpp>
#include <Auth/AuthScene.hpp>
#include <VirtualConsole/VirtualConsole.hpp>
#include <System/SystemDisplay.hpp>

void Application::InitOnSystemModeChanged()
{
	if (!SystemModeChanged())
		return;

	SystemMode mode = GetSystemMode();

	PriorityTaskSheduler::Clear();
	SceneManager::Instance().ClearScenes();

	SystemAllocator.Clear();
	CommonAllocator.Clear();

	InitSystemTasks();

	if (mode == SystemMode::AUTH)
	{
		SceneManager::Instance().AddSceneBuilder<AuthScene>(1);
		SceneManager::Instance().SwitchScene(1);
		VirtualConsole::Instance().Print("Auth Init", GreenColor);
	}
	else if (mode == SystemMode::ADMIN)
	{
		InitSystemScenes();
		VirtualConsole::Instance().Print("Admin Init", GreenColor);
	}
	else if (mode == SystemMode::USER)
	{
		InitUserTasks();
		InitUserScenes();

		Account acc;
		if (AuthHandler::Instance().GetCurrentAccount(acc))
			VirtualConsole::Instance().Printf("User %d Init", GreenColor, acc.ID);
		else
		{
			VirtualConsole::Instance().Print("User Init", GreenColor);
			VirtualConsole::Instance().Print("Get ID failed", RedColor);
		}
	}
}

#include <Time/DateTime.hpp>
#include <DrawFunctions/DrawText.hpp>
#include <Graphics/DefaultFont.hpp>
#include <System/DebugMode.hpp>
#include <VirtualConsole/VirtualConsole.hpp>

#ifndef USE_HAL_DRIVER

#include <FatFs/low_level/ff_win.hpp>

#else

#include <stdarg.h>
#include <FatFs/low_level/ff.h>
#include <SDCard/sdcard.h>
#include <st7735/low_level/st7735.h>


#endif

#include <FatFs/FatFsTest.hpp>
#include <System/DeviceID.hpp>

#include "ProjectScenes/SceneID.hpp"

void Application::Init()
{
//	__HAL_TIM_CLEAR_FLAG(htim, TIM_SR_UIF);
//	HAL_TIM_Base_Start_IT(htim);
	printf("init\n");

	SystemDisplay = &display;
	DisplayStackGuard = &display_guard;
	
	InitExtDev();

#ifndef USE_HAL_DRIVER
	fatfs_image_initialize(arg_parser.FindArg("--client") ? "client.img" : "server.img");
#else 
	/// TODO: Выделить SDCARD_Init в метод сущности SDCARD
	// unselect all SPI devices first

    // initialize SD-card as fast as possible, it glitches otherwise
    // (this is important if SPI bus is shared by multiple devices)

    // Перед инициализацией установите низкую частоту:
    //SDCARD_SetSpeed(SPI_BAUDRATEPRESCALER_256);
//
    //int code = SDCARD_Init();
//
    //// После успешной инициализации можно увеличить скорость:
    //SDCARD_SetSpeed(SPI_BAUDRATEPRESCALER_2);
//
    //if(code < 0) 
	//{
    //	VC_Printf("SDCARD_Init() failed", RedColor);
    //	VC_Printf("code = %d\r\n", RedColor, code);
    //	VC_Printf("R1 = %d\r\n", BlueColor, SDCARD_LastReadR1());
    //}
#endif

	// Инициализация настроек обязательно должна идти перед остальными,
	// поскольку на них может опираться инициализации периферии и прочего
	SystemStorage.Init(eeprom_24c_08);

	SetDeviceID(SerialNumber.GetOrDefault());
	InitDebugModeCell(&DebugMode);

	// Инициализация обработчика аутентификации и авторизации
	AuthHandler::Instance().Init(AuthHandlerStart, SystemStorage, 7, 0x0400, 0x0400);
	
	// Подсчет количества запусков
	StartsCount.Set(StartsCount.GetOrDefault() + 1);

	TimeUtils::Init(10000);

#ifndef USE_HAL_DRIVER
	TimeUtils::SetStartMs(TimeUtils::GetCurrentMs());

	FileConfig::SetConfigRootPath();
	FileConfig::SetBool("system", "reboot", false);
#else
	DateTime::Init(hrtc);

	if (UARTConsoleOnStart.GetOrDefault())
		uart_log_init(huart_console);

	static I2CCom 		i2c_com		("i2c", "i2c command", hi2c);
	static NRFCom 		nrf_com		("nrf", "nrf command", huart_console);
	static PrintCommand print_com	("print", "dummy command");
	static AllocCom 	alloc_com	("alloc", "alloc info");

	CommandHandler::RegConsoleCommand(&i2c_com);
	CommandHandler::RegConsoleCommand(&nrf_com);
	CommandHandler::RegConsoleCommand(&print_com);
	CommandHandler::RegConsoleCommand(&alloc_com);
#endif

	DrawFunctions::SetDefaultFont(&Default_Font_8x8);

	display.Init();

	// Инициалиация пользовательского ввода
	InitInput();

	// Установить режим аутентификации
	SetSystemMode(SystemMode::AUTH);

#ifndef USE_HAL_DRIVER

	if (arg_parser.FindArg("--user"))
	{
		std::string auth_idx = arg_parser.FindValue("--user", "0");
		int idx = auth_idx[0] - '0';
		
		int cnt_idx = 0;
		for (const Account& account : AuthHandler::Instance().GetAccounts())
		{
			if (cnt_idx == idx)
			{
				AuthHandler::Instance().Login(account.ID);
				SetSystemMode(SystemMode::USER);
				InitOnSystemModeChanged();
				SceneManager::Instance().SwitchScene(SceneID::MAIN);
				SceneManager::Instance().Loop();
				break;
			}
			cnt_idx++;
		}

		if (arg_parser.FindArg("--game"))
		{
			SceneManager::Instance().SwitchScene(SceneID::GAMES_CENTER);
		}
	}

#endif
}

#include <NRF24L01/NRF24L01_data.hpp>

bool Application::Loop()
{
	SystemStackGuardLoop();
	PriorityTaskSheduler::Shedule();
	NRF24L01::Scanner.Loop();

#ifdef __LINUX__
	qapp->processEvents();
#endif

	// Если изменился режим работы системы, производится переинициализация
	InitOnSystemModeChanged();

	bool loopOK = true;
	loopOK &= display.IsOpen();
	loopOK &= !rebootFlag;
	return loopOK;
}

bool Application::GetRebootFlag() const
{
	return rebootFlag;
}

#include <Input/Devices/DiscreteSignalDevice.hpp>
#include <Input/Devices/ButtonDevice.hpp>
#include <Input/Devices/EncoderDevice.hpp>
#include <Input/Devices/UserInputDevice.hpp>

#include <Input/SystemInputID.hpp>

void Application::InitInput()
{
	static bool inited = false;
	if (inited)
		return;
	inited = true;

//// Инициализация дискретного сигнала 1
//	DiscreteSignal* ds1 = new DiscreteSignal(1);
//	UserInputs[UserInputsCount++] = ds1;
//#ifdef USE_HAL_DRIVER
//	ds1->Init("ds1", PORT_BTN_2_GPIO_Port, PORT_BTN_2_Pin);
//#else
//	ds1->Init("ds1");
//#endif

	// Создание объектов кнопок
	static ButtonDevice button[7]{};

	// Инициализация кнопок
#ifdef USE_HAL_DRIVER
	button[0].Init(SYSTEM_BTN_UP, BTN_1_GPIO_Port, BTN_1_Pin);
	button[1].Init(SYSTEM_BTN_DOWN, BTN_2_GPIO_Port, BTN_2_Pin);
	button[2].Init(SYSTEM_BTN_LEFT, BTN_3_GPIO_Port, BTN_3_Pin);
	button[3].Init(SYSTEM_BTN_RIGHT, BTN_4_GPIO_Port, BTN_4_Pin);
	button[4].Init(SYSTEM_BTN_ENTER, BTN_5_GPIO_Port, BTN_5_Pin);
	button[5].Init(SYSTEM_BTN_MENU, BTN_6_GPIO_Port, BTN_6_Pin);
	button[6].Init(SYSTEM_BTN_ALT, BTN_7_GPIO_Port, BTN_7_Pin);
#else
	constexpr int Left_key = 37;
	constexpr int Up_key = 38;
	constexpr int Right_key = 39;
	constexpr int Down_key = 40;
	constexpr int Enter_key = 13;
	constexpr int Escape_key = 27;
	constexpr int Space_key = 32;

	button[0].Init(SYSTEM_BTN_UP, Up_key);
	button[1].Init(SYSTEM_BTN_DOWN, Down_key);
	button[2].Init(SYSTEM_BTN_LEFT, Left_key);
	button[3].Init(SYSTEM_BTN_RIGHT, Right_key);
	button[4].Init(SYSTEM_BTN_ENTER, Enter_key);
	button[5].Init(SYSTEM_BTN_MENU, Escape_key);
	button[6].Init(SYSTEM_BTN_ALT, Space_key);
#endif

	for (uint8_t i = 0; i < sizeof(button) / sizeof(button[0]); ++i)
	{
		UserInputs.Push(&button[i]);
	}

	// Создание и инициализация энкодера
	static EncoderDevice encoder1{};

#ifdef USE_HAL_DRIVER
	encoder1.Init(SYSTEM_ENC_MAIN, ENC_CLK_GPIO_Port, ENC_CLK_Pin, ENC_DT_GPIO_Port, ENC_DT_Pin);
#else
	encoder1.Init(SYSTEM_ENC_MAIN, 189, 187);
#endif

	encoder1.SetReverse(EncoderReverse.GetOrDefault());

	UserInputs.Push(&encoder1);

	StackGuard guard{};
	
	SystemStackGuard = (StackGuard*) malloc(sizeof(StackGuard));
	memcpy(SystemStackGuard, &guard, sizeof(StackGuard));

	//SystemStackGuard1 = new StackGuard;

	//#define TEST_STACK_GUARD

#if !defined(USE_HAL_DRIVER) && defined(TEST_STACK_GUARD)
	uint32_t* ptr = (uint32_t*) SystemStackGuard1;
	ptr += 40;

	memset(ptr, 0, 4);
#endif
}

#include <SDCard/SDCard_ExtDevice.hpp>
#include <st7735/ST7735_ExtDevice.hpp>
#include <NRF24L01/nRF24L01_ExtDevice.hpp>

void Application::InitExtDev()
{
	{
		static SDCard_ExtDevice dev;
		sdcard_extdev = &dev;
		ExtDevices.push_back(&dev);
	}
	{
		static ST7735_ExtDevice dev;
		st7735_extdev = &dev;
		ExtDevices.push_back(&dev);
	}
	{
		static nRF24L01_ExtDevice dev;
		nrf24l01_extdev = &dev;
		ExtDevices.push_back(&dev);
	}
}

#include "ProjectScenes/DebugScene.hpp"
#include "ProjectScenes/EEPROMScene.hpp"
#include "ProjectScenes/MainScene.hpp"
#include "ProjectScenes/WelcomeScene.hpp"
#include "ProjectScenes/EyesScene.hpp"
#include "ProjectScenes/SystemScene.hpp"
#include "ProjectSettings/ProjectSettingsScene.hpp"
#include "ProjectSettings/ProjectSettingsScene2.hpp"
#include <GamesSupport/GameCenter.hpp>

//#include <Battleship/Make.hpp>
//#include <LifeGame/Make.hpp>
//#include <PingPong/Make.hpp>
//#include <TicTacToe/Make.hpp>
//#include <Tanks/Make.hpp>

#include <Auth/Account_EditScene.hpp>
#include <Auth/Account_DeleteScene.hpp>
#include <Auth/Account_SettingScene.hpp>

#include <System/CommonAllocator.hpp>

void Application::InitUserScenes()
{
	// ============================== MAIN SCENES ==============================

	// Кадр с настройками 
	SceneManager::Instance().AddSceneBuilder<ProjectSettingsScene>(SceneID::SETTINGS);

	// Кадр с задачами (системный)
	SceneManager::Instance().AddSceneBuilder<SystemScene>(SceneID::SYSTEM);

	// Изменение аккаунта
	SceneManager::Instance().AddSceneBuilder<Account_EditScene>(SceneID::EDIT_ACCOUNT);

	// Удаление аккаунта
	SceneManager::Instance().AddSceneBuilder<Account_DeleteScene>(SceneID::DELETE_ACCOUNT);

	// Настройка аккаунта
	SceneManager::Instance().AddSceneBuilder<Account_SettingScene>(
		SceneID::SETTING_ACCOUNT, 
		(uint32_t) SceneID::EDIT_ACCOUNT, 
		(uint32_t) SceneID::DELETE_ACCOUNT);

	// Игровой центр
	SceneManager::Instance().AddSceneBuilder<GameCenter>(SceneID::GAMES_CENTER);

	// ===========================================================================

	// Кадр с выбором других кадров
	SceneManager::Instance().AddSceneBuilder<MainScene>(SceneID::MAIN);

	// Кадр приветствия
	SceneManager::Instance().AddSceneBuilder<WelcomeScene>(SceneID::WELCOME);

	// Кадр с "глазками"
	SceneManager::Instance().AddSceneBuilder<EyesScene>(SceneID::EYES);

	SceneManager::Instance().SwitchScene(SceneID::WELCOME);
}


// ============================== Инициализация задач ==============================

#include "Common/ProjectGraphs.hpp"

#include <NRF24L01/nRF24L01_ReadTask.hpp>

class ResearchEncoderTask : public PriorityTask
{
public:
	ResearchEncoderTask(StaticText32 name, uint32_t period_ms, uint8_t priority)
		: PriorityTask{ name, period_ms, priority }
	{
	}

	bool Do() override
	{
		bool clk = false, dt = false;
#ifdef USE_HAL_DRIVER
		clk = !HAL_GPIO_ReadPin(ENC_CLK_GPIO_Port, ENC_CLK_Pin);
		dt = !HAL_GPIO_ReadPin(ENC_DT_GPIO_Port, ENC_DT_Pin);
#endif

		DEBUG_ENCODER_CLK.AddValue(clk);
		DEBUG_ENCODER_DT.AddValue(dt);

		return true;
	}

};

#include <System/SystemTestScene.hpp>

void Application::InitSystemScenes()
{
	// ============================== MAIN SCENES ==============================

	// Кадр "EEPROM"
	SceneManager::Instance().AddSceneBuilder<EEPROMScene>(SceneID::EEPROM, &eeprom_24c_08);

	// Кадр "Debug"
	SceneManager::Instance().AddSceneBuilder<DebugScene>(SceneID::DEBUG_SCENE);

	// Кадр с задачами (системный)
	SceneManager::Instance().AddSceneBuilder<SystemScene>(SceneID::SYSTEM);

	// Кадр с настройками 
	SceneManager::Instance().AddSceneBuilder<ProjectSettingsScene>(SceneID::SETTINGS);

	// Кадр с настройками №2
	SceneManager::Instance().AddSceneBuilder<ProjectSettingsScene2>(SceneID::SETTINGS2);

	// Кадр "Тест"
	SceneManager::Instance().AddSceneBuilder<SystemTestScene>(SceneID::TEST_SCENE);

	// ===========================================================================

	// Кадр с выбором других кадров
	SceneManager::Instance().AddSceneBuilder<MainScene>(SceneID::MAIN);

	SceneManager::Instance().SwitchScene(SceneID::MAIN);
}

#include <Time/DateTime.hpp>
#include <FatFs/FatFsTask.hpp>
#include <ExtDevice/ExtDeviceProcessor.hpp>

void Application::InitSystemTasks()
{
	// Под HAL пока ничего не делает, под WIN обрабатывает входящие события окна
	PriorityTaskSheduler::AddTask<IDisplay_LoopTask>("SCRN_L", 10, 45, &display);
	PriorityTaskSheduler::AddTask<IDisplay_RenderTask>("SCRN_D", 10, 35, &display);

	PriorityTaskSheduler::AddTask<UserInputTickTask>("INPUT", 4, 50, 		StaticListView<UserInputDevice*>{ UserInputs });
	PriorityTaskSheduler::AddTask<UserInputPopEventTask>("INPUT2", 20, 15, 	StaticListView<UserInputDevice*>{ UserInputs });

	PriorityTaskSheduler::AddTask<ResearchEncoderTask>("ENC_TSK", 10, 40);
	PriorityTaskSheduler::AddTask<nRF24L01_ReadTask>("NRF_ST", 1000, 10);
	PriorityTaskSheduler::AddTask<FatFsTask>("FF_TSK", 3000, 25, sdcard_extdev);

	// TODO: можно попробовать поднять частоту опроса (но зачем? - чтобы дискретность изменений была ниже)
	PriorityTaskSheduler::AddTask<DateTimeServer>("DT_SERV", 10, 50);

	ExtDeviceProcessor* extdev_proc = PriorityTaskSheduler::AddTask<ExtDeviceProcessor>("EXTDEV_P", 200, 50);
	for (ExtDevice* dev : ExtDevices)
		extdev_proc->AddExtDevice(dev);
}

#include <Web/WebDevicesTask.hpp>

#include <LetoAPI_V1_System/Web/WebManager.hpp>
#include <LetoAPI_V1_System/Lobby/LobbyTask.hpp>

void Application::InitUserTasks()
{
	// Высокий приоритет + редкая выдача
	WebDevicesTask* web_devices_task = PriorityTaskSheduler::AddTask<WebDevicesTask>("SYN_DEVS", 500, 50);

	WebQueueRxTxTask* web = PriorityTaskSheduler::AddTask<WebQueueRxTxTask>("WEB", 50, 25);
#ifdef USE_HAL_DRIVER
	web->Init(huart_web);
#elif defined(_WIN32)
	bool server = !arg_parser.FindArg("--client") && !arg_parser.FindArg("-c");
	web->Init(server, "web");
#endif

	PriorityTaskSheduler::AddTask<LobbyTask>("LOBBY", 50, 30);
}
