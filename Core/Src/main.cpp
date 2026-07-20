/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "project_includes.hpp"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CRC_HandleTypeDef hcrc;

I2C_HandleTypeDef hi2c1;
DMA_HandleTypeDef hdma_i2c1_tx;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi3;
DMA_HandleTypeDef hdma_spi1_tx;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart2;
UART_HandleTypeDef huart6;

/* USER CODE BEGIN PV */
Application::Periphery periphery =
{
	.hi2c = &hi2c1,
	.htim = nullptr,
	.huart_console = &huart2,
	.huart_web = &huart6,
	.hrtc = &hrtc,
};
Application app{ periphery };
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM1_Init(void);
static void MX_USART6_UART_Init(void);
static void MX_CRC_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI3_Init(void);
static void MX_SPI1_Init(void);
/* USER CODE BEGIN PFP */
#include <UART/UART_printf_prototype.hpp>
#include <UART/UART_console_init.hpp>
#include <CommandHandler/CommandHandler.hpp>
#include <LetoAPI_V1_System/Web/WebPacketParser.hpp>
#include <LetoAPI_V1/Web/WebConnection.hpp>
//#include <NRF24L01/nrf24l01.h>
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static uint8_t buffer2, buffer6;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART2)
	{
		if (!uart_log_enabled)
			return;

		HAL_UART_Receive_IT(huart, &buffer2, 1);

        if((buffer2 == '\x08')||(buffer2 == '\x7F'))  // backspace
        {
        	const char del[] = "\x08 \x08";
			HAL_UART_Transmit(huart, (uint8_t*)del, sizeof(del)-1, 1000);
        }
        else if((buffer2 == '\r') || (buffer2 == '\n'))
        {
        	const char nl[] = "\r\n";
            HAL_UART_Transmit(huart, (uint8_t*)nl, sizeof(nl)-1, 1000);
        }
        else
        {
    		HAL_UART_Transmit(huart, &buffer2, 1, 1000);
        }

		CommandHandler::Callback((char) buffer2);
	}
	else if (huart->Instance == USART6)
	{
		HAL_UART_Receive_IT(huart, &buffer6, 1);
		WebPacketParser::CallbackByte(WC_CHANNEL_UART, buffer6);
	}
}
/* USER CODE END 0 */

#include "ProjectSettings/ProjectSettings.hpp"
#include <NRF24L01/NRF24L01_data.hpp>

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
   HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_USART6_UART_Init();
  MX_CRC_Init();
  MX_RTC_Init();
  MX_SPI3_Init();
  MX_SPI1_Init();
  /* USER CODE BEGIN 2 */
#if defined(USE_HAL_DRIVER)
  DWT_Init(); // счётчик для микросекундных пауз

  uint8_t res = isChipConnected(); // проверяет подключён ли модуль к SPI

  char str[64] = {0,};
  snprintf(str, 64, "Connected: %s\n\r", 1 ? "OK" : "NOT OK");
  HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), 1000);

  res = NRF_Init(); // инициализация

  snprintf(str, 64, "Init: %s\n\r", res > 0 && res < 255 ? "OK" : "NOT OK");
  HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), 1000);

  snprintf(str, 64, "P-variant: %d\n\r", isPVariant());
  HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), 1000);

  ////////////// SET ////////////////
  //enableAckPayload();
  setAddressWidth(4); // Я ДОБАВИЛ
  setAutoAck(true);
  //setPayloadSize(3);
  setChannel(32);
  // Delay: 1 (500mks), count: 5
  setRetries(1, 5);

  ///////////////////////////////////

  ////////////////////////// Вывод всяких статусов, для работы не нужно /////////////////////////////
  uint8_t status = get_status();
  snprintf(str, 64, "get_status: 0x%02x\n\r", status);
  HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), 1000);

  status = getPALevel();
  snprintf(str, 64, "getPALevel: 0x%02x  ", status);
  HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), 1000);

  if(status == 0x00)
  {
	  HAL_UART_Transmit(&huart2, (uint8_t*)"RF24_PA_MIN\n\r", strlen("RF24_PA_MIN\n\r"), 1000);
  }
  else if(status == 0x01)
  {
	  HAL_UART_Transmit(&huart2, (uint8_t*)"RF24_PA_LOW\n\r", strlen("RF24_PA_LOW\n\r"), 1000);
  }
  else if(status == 0x02)
  {
	  HAL_UART_Transmit(&huart2, (uint8_t*)"RF24_PA_HIGH\n\r", strlen("RF24_PA_HIGH\n\r"), 1000);
  }
  else if(status == 0x03)
  {
	  HAL_UART_Transmit(&huart2, (uint8_t*)"RF24_PA_MAX\n\r", strlen("RF24_PA_MAX\n\r"), 1000);
  }

  status = getChannel();
  snprintf(str, 64, "getChannel: 0x%02x № %d\n\r", status, status);
  HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), 1000);

  status = getDataRate();
  snprintf(str, 64, "getDataRate: 0x%02x  ", status);
  HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), 1000);

  if(status == 0x02)
  {
	  HAL_UART_Transmit(&huart2, (uint8_t*)"RF24_250KBPS\n\r", strlen("RF24_250KBPS\n\r"), 1000);
  }
  else if(status == 0x01)
  {
	  HAL_UART_Transmit(&huart2, (uint8_t*)"RF24_2MBPS\n\r", strlen("RF24_2MBPS\n\r"), 1000);
  }
  else
  {
	  HAL_UART_Transmit(&huart2, (uint8_t*)"RF24_1MBPS\n\r", strlen("RF24_1MBPS\n\r"), 1000);
  }

  status = getPayloadSize();
  snprintf(str, 64, "getPayloadSize: %d\n\r", status);
  HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), 1000);

  status = getCRCLength();
  snprintf(str, 64, "getCRCLength: 0x%02x  ", status);
  HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), 1000);

  if(status == 0x00)
  {
	  HAL_UART_Transmit(&huart2, (uint8_t*)"RF24_CRC_DISABLED\n\r", strlen("RF24_CRC_DISABLED\n\r"), 1000);
  }
  else if(status == 0x01)
  {
	  HAL_UART_Transmit(&huart2, (uint8_t*)"RF24_CRC_8\n\r", strlen("RF24_CRC_8\n\r"), 1000);
  }
  else if(status == 0x02)
  {
	  HAL_UART_Transmit(&huart2, (uint8_t*)"RF24_CRC_16\n\r", strlen("RF24_CRC_16\n\r"), 1000);
  }
  /////////////////////////////////////////////////////////////////////////////////////////////////////


  maskIRQ(true, true, true); // маскируем прерывания
#endif
  app.Init();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_UART_Receive_IT(&huart2, &buffer2, 1);
  HAL_UART_Receive_IT(&huart6, &buffer6, 1);
#if defined(USE_HAL_DRIVER)
  while (1)
  {
	  app.Loop();

	    /* USER CODE END WHILE */

	    /* USER CODE BEGIN 3 */
  }
#elif defined(USE_HAL_DRIVER)
  openReadingPipe(1, NRF24L01::PipeAddress[3]);
  startListening();

  while (1)
  {
	///////////////////////////////////// ПРИЁМ /////////////////////////////////////////////
	uint8_t nrf_data[32] = {0,}; // буфер указываем максимального размера
	//static uint8_t remsg = 0;
	uint8_t pipe_num = 0;

	while (available(&pipe_num) && isPowerUp()) // проверяем пришло ли что-то
	{
		//remsg++;

		//writeAckPayload(pipe_num, &remsg, sizeof(remsg)); // отправляем полезную нагрузку вместе с подтверждением

		uint8_t count = getDynamicPayloadSize(); // смотрим сколько байт прилетело
		read(&nrf_data, count); // Читаем данные в массив nrf_data и указываем сколько байт читать

		if(pipe_num == 0) // проверяем в какую трубу пришли данные
		{
			HAL_UART_Transmit(&huart2, (uint8_t*)"pipe 0\n\r", strlen("pipe 0\n\r"), 1000);
		}

		else if(pipe_num == 1)
		{
			HAL_UART_Transmit(&huart2, (uint8_t*)"pipe 1\n\r", strlen("pipe 1\n\r"), 1000);

			if (count > 14)
			{
				snprintf(str, 64, "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n\r",
						nrf_data[0], nrf_data[1], nrf_data[2],
						nrf_data[3], nrf_data[4], nrf_data[5],
						nrf_data[6], nrf_data[7], nrf_data[8],
						nrf_data[9], nrf_data[10], nrf_data[11],
						nrf_data[12], nrf_data[13], nrf_data[14]);
				HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), 1000);
			}
		}

		else if(pipe_num == 2)
		{
			HAL_UART_Transmit(&huart2, (uint8_t*)"pipe 2\n\r", strlen("pipe 2\n\r"), 1000);
		}

		else
		{
			//while(availableMy()) // если данные придут от неуказанной трубы, то попадут сюда
			//{
				//read(&nrf_data, sizeof(nrf_data));
				HAL_UART_Transmit(&huart2, (uint8_t*)"Unknown pipe\n", strlen("Unknown pipe\n"), 1000);
			//}
		}
	}
	HAL_Delay(10000);

	stopListening();
	openWritingPipe(NRF24L01::PipeAddress[1]);
    bool w = write((uint8_t*)"HELLO WORLD", sizeof("HELLO WORLD")); // отправляем данные
    printf("write result: %d\n", w);
	startListening();

	HAL_Delay(10000);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
#else
  stopListening();
  openWritingPipe(NRF24L01::PipeAddress[3]);

  while (1)
  {
	  ///////////////////////////////////// ПЕРЕДАЧА /////////////////////////////////////////////
	  uint8_t nrf_data[32] = {0,}; // буфер указываем максимального размера
	  nrf_data[0] = 77;
	  nrf_data[1] = 86;
	  nrf_data[2] = 97;

	  if(write(&nrf_data, strlen((const char*)nrf_data))) // отправляем данные
			HAL_UART_Transmit(&huart2, (uint8_t*)"Write!!!\n\r", strlen("Write!!!\n\r"), 1000);
	  else
			HAL_UART_Transmit(&huart2, (uint8_t*)"Not write\n\r", strlen("Not write\n\r"), 1000);

	  HAL_Delay(1000);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }

#endif
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{

  /* USER CODE BEGIN CRC_Init 0 */

  /* USER CODE END CRC_Init 0 */

  /* USER CODE BEGIN CRC_Init 1 */

  /* USER CODE END CRC_Init 1 */
  hcrc.Instance = CRC;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CRC_Init 2 */

  /* USER CODE END CRC_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */
  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
  return; // Остальное инициализируем сами
  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}
/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI3_Init(void)
{

  /* USER CODE BEGIN SPI3_Init 0 */

  /* USER CODE END SPI3_Init 0 */

  /* USER CODE BEGIN SPI3_Init 1 */

  /* USER CODE END SPI3_Init 1 */
  /* SPI3 parameter configuration*/
  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI3_Init 2 */

  /* USER CODE END SPI3_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 19200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART6_UART_Init(void)
{

  /* USER CODE BEGIN USART6_Init 0 */

  /* USER CODE END USART6_Init 0 */

  /* USER CODE BEGIN USART6_Init 1 */

  /* USER CODE END USART6_Init 1 */
  huart6.Instance = USART6;
  huart6.Init.BaudRate = 115200;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART6_Init 2 */

  /* USER CODE END USART6_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(PORT_LED_GPIO_Port, PORT_LED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI1_DC_GPIO_Port, SPI1_DC_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SPI1_CS_Pin|SPI1_RST_Pin|SPI1_SD_CS_Pin|CSN_Pin
                          |CE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PORT_LED_Pin */
  GPIO_InitStruct.Pin = PORT_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PORT_LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI1_DC_Pin */
  GPIO_InitStruct.Pin = SPI1_DC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI1_DC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SPI1_CS_Pin SPI1_SD_CS_Pin */
  GPIO_InitStruct.Pin = SPI1_CS_Pin|SPI1_SD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : SPI1_RST_Pin CSN_Pin CE_Pin */
  GPIO_InitStruct.Pin = SPI1_RST_Pin|CSN_Pin|CE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : BTN_6_Pin ENC_CLK_Pin ENC_DT_Pin BTN_5_Pin
                           BTN_2_Pin */
  GPIO_InitStruct.Pin = BTN_6_Pin|ENC_CLK_Pin|ENC_DT_Pin|BTN_5_Pin
                          |BTN_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : BTN_1_Pin BTN_4_Pin BTN_3_Pin BTN_7_Pin */
  GPIO_InitStruct.Pin = BTN_1_Pin|BTN_4_Pin|BTN_3_Pin|BTN_7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}
/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
