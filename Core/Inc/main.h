/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define PORT_LED_Pin GPIO_PIN_13
#define PORT_LED_GPIO_Port GPIOC
#define SPI1_DC_Pin GPIO_PIN_4
#define SPI1_DC_GPIO_Port GPIOA
#define SPI1_CS_Pin GPIO_PIN_0
#define SPI1_CS_GPIO_Port GPIOB
#define SPI1_RST_Pin GPIO_PIN_1
#define SPI1_RST_GPIO_Port GPIOB
#define SPI1_SD_CS_Pin GPIO_PIN_2
#define SPI1_SD_CS_GPIO_Port GPIOB
#define BTN_6_Pin GPIO_PIN_10
#define BTN_6_GPIO_Port GPIOB
#define BTN_7_Pin GPIO_PIN_15
#define BTN_7_GPIO_Port GPIOA
#define ENC_CLK_Pin GPIO_PIN_12
#define ENC_CLK_GPIO_Port GPIOB
#define ENC_DT_Pin GPIO_PIN_13
#define ENC_DT_GPIO_Port GPIOB
#define BTN_5_Pin GPIO_PIN_14
#define BTN_5_GPIO_Port GPIOB
#define BTN_2_Pin GPIO_PIN_15
#define BTN_2_GPIO_Port GPIOB
#define BTN_1_Pin GPIO_PIN_8
#define BTN_1_GPIO_Port GPIOA
#define BTN_4_Pin GPIO_PIN_9
#define BTN_4_GPIO_Port GPIOA
#define BTN_3_Pin GPIO_PIN_10
#define BTN_3_GPIO_Port GPIOA
#define CSN_Pin GPIO_PIN_6
#define CSN_GPIO_Port GPIOB
#define CE_Pin GPIO_PIN_7
#define CE_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
