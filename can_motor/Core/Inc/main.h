/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#define LED_B_Pin GPIO_PIN_12
#define LED_B_GPIO_Port GPIOH
#define LED_G_Pin GPIO_PIN_11
#define LED_G_GPIO_Port GPIOH
#define LED_R_Pin GPIO_PIN_10
#define LED_R_GPIO_Port GPIOH

/* USER CODE BEGIN Private defines */
/* ---------- C610 电调 / M2006 电机 参数（取自《C610 无刷电机调速器使用说明》） ---------- */
#define C610_CTRL_ID_1_4      0x200u  /* 控制帧标识符：控制 ID 1~4 号电调 */
#define C610_CTRL_ID_5_8      0x1FFu  /* 控制帧标识符：控制 ID 5~8 号电调 */
#define C610_FEEDBACK_ID_BASE 0x200u  /* 反馈帧标识符 = 0x200 + 电调 ID（ID=1 -> 0x201） */
#define MOTOR_ID              1u      /* 电调实际 ID：上电后绿灯每秒闪 N 次，N 就是当前 ID */
#define MOTOR_CURRENT         1000    /* 控制电流值 -10000~10000 对应 -10~10A */
#define CTRL_PERIOD_MS        10u     /* 10ms 发一帧 = 100Hz 控制频率 */
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
