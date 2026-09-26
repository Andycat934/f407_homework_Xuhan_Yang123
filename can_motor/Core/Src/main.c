/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
CAN_HandleTypeDef hcan1;

/* USER CODE BEGIN PV */
/* 收到过多少帧电调反馈：用来让绿灯以肉眼看得见的速度闪
   （反馈默认 1kHz，累计 500 帧翻转一次 ≈ 2Hz） */
static uint32_t rx_frame_cnt = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_CAN1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

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
  MX_CAN1_Init();
  /* USER CODE BEGIN 2 */
  /* ============================================================
   * TODO 1/2/3/4/5 的答案：C610 控制帧的帧头
   *   说明书：标识符 0x200（ID 1~4）/ 0x1FF（ID 5~8）
   *           帧类型 标准帧、帧格式 DATA、DLC 8 字节
   * ============================================================ */
  CAN_TxHeaderTypeDef txHeader = {0};
  txHeader.StdId = C610_CTRL_ID_1_4;   /* 0x200：控制 ID 1~4 号电调（原来写的 0x713 是别的设备的 ID） */
  txHeader.ExtId = 0;                  /* 标准帧不使用扩展 ID */
  txHeader.IDE = CAN_ID_STD;           /* 帧类型：标准帧（不是 CAN_ID_EXT 扩展帧） */
  txHeader.RTR = CAN_RTR_DATA;         /* 帧格式：DATA（不是 CAN_RTR_REMOTE 远程帧） */
  txHeader.DLC = 8;                    /* DLC：8 字节（不是 6） */
  txHeader.TransmitGlobalTime = DISABLE;

  /* ============================================================
   * TODO 6：构造控制电机的 CAN 帧
   *   每台电调占 2 个字节，高位在前（大端 int16）：
   *   DATA[0..1]=ID1  DATA[2..3]=ID2  DATA[4..5]=ID3  DATA[6..7]=ID4
   *   本实验总线上只有一台电调（ID=1），所以只填 DATA[0..1]，其余保持 0
   * ============================================================ */
  uint8_t txData[8] = {0};
  int16_t current = MOTOR_CURRENT;              /* 1000 -> 约 1A 转矩电流 */
  txData[0] = (uint8_t)((current >> 8) & 0xFF); /* 控制电流值高 8 位 */
  txData[1] = (uint8_t)(current & 0xFF);        /* 控制电流值低 8 位 */

  uint32_t txMailbox;
  uint32_t last_tx_tick = HAL_GetTick();
  uint32_t heartbeat = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* ==========================================================
     * TODO 7：控制频率 100Hz（原来 HAL_Delay(500) 只有 2Hz）。
     * 电调靠“周期性收到控制帧”维持输出，停发就会停机；
     * 用 HAL_GetTick() 判断时间到没到，比 HAL_Delay 更稳。
     * ========================================================== */
    if ((HAL_GetTick() - last_tx_tick) >= CTRL_PERIOD_MS)
    {
      last_tx_tick = HAL_GetTick();

      if (HAL_CAN_AddTxMessage(&hcan1, &txHeader, txData, &txMailbox) != HAL_OK)
      {
        /* 3 个发送邮箱都满 / 总线错误：蓝灯亮起提示（这里不能死等） */
        HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_SET);
      }
      else
      {
        HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin, GPIO_PIN_RESET);

        /* 红灯心跳：每 50 帧（0.5s）翻转一次，说明主循环还在跑 */
        if (++heartbeat >= (500u / CTRL_PERIOD_MS))
        {
          heartbeat = 0;
          HAL_GPIO_TogglePin(LED_R_GPIO_Port, LED_R_Pin);
        }
      }
      /* 绿灯在 HAL_CAN_RxFifo0MsgPendingCallback() 里翻转：每收到一帧电调反馈闪一次 */
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 6;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 3;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_10TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_3TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = DISABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */
  /* 光有 HAL_CAN_Init() 只是把 CAN 寄存器配好，还开不了工：必须再
     （1）配过滤器  （2）Start  （3）打开 RX 中断通知，否则一帧都收不到。
     掩码全 0 => 任何 ID 都放行，并且都丢进 FIFO0（回调是 RxFifo0 版本） */
  CAN_FilterTypeDef can_filter_st = {0};
  can_filter_st.FilterActivation = ENABLE;
  can_filter_st.FilterMode = CAN_FILTERMODE_IDMASK;
  can_filter_st.FilterScale = CAN_FILTERSCALE_32BIT;
  can_filter_st.FilterIdHigh = 0x0000;
  can_filter_st.FilterIdLow = 0x0000;
  can_filter_st.FilterMaskIdHigh = 0x0000;
  can_filter_st.FilterMaskIdLow = 0x0000;
  can_filter_st.FilterBank = 0;              /* 第 0 组过滤器属于 CAN1 */
  can_filter_st.FilterFIFOAssignment = CAN_RX_FIFO0;
  can_filter_st.SlaveStartFilterBank = 14;   /* 0~13 归 CAN1，14~27 归 CAN2 */
  if (HAL_CAN_ConfigFilter(&hcan1, &can_filter_st) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_CAN_Start(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE END CAN1_Init 2 */

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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOH, LED_B_Pin|LED_G_Pin|LED_R_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED_B_Pin LED_G_Pin LED_R_Pin */
  GPIO_InitStruct.Pin = LED_B_Pin|LED_G_Pin|LED_R_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* 开发板 C 型的三色 LED：PH10=红 PH11=绿 PH12=蓝，
     经 NPN 三极管驱动，引脚输出高电平 -> 灯亮。
     （原来翻转的 PE5 是 F407VGT6 最小系统板的灯，C 板上没有接 LED） */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
/**
  * @brief  FIFO0 里有未读帧时，HAL_CAN_IRQHandler() 会调用这个弱函数（在 main.c 里重写）
  * @note   中断里不要调用 Error_Handler()：它会 __disable_irq() 然后死循环
  * @note   每次进中断都要把 FIFO0 读空，否则 pending 中断会一直有效，卡死主循环
  */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  CAN_RxHeaderTypeDef rx_header;
  uint8_t rx_data[8];

  while (HAL_CAN_GetRxFifoFillLevel(hcan, CAN_RX_FIFO0) > 0)
  {
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK)
    {
      break;
    }

    /* 只关心自己这台电调的反馈帧：标识符 = 0x200 + 电调 ID */
    if ((rx_header.IDE == CAN_ID_STD) &&
        (rx_header.StdId == (C610_FEEDBACK_ID_BASE + MOTOR_ID)))
    {
      int16_t rotor_angle   = (int16_t)((rx_data[0] << 8) | rx_data[1]);  /* 0~8191 对应 0~360° */
      int16_t rotor_speed   = (int16_t)((rx_data[2] << 8) | rx_data[3]);  /* rpm */
      int16_t real_current  = (int16_t)((rx_data[4] << 8) | rx_data[5]);  /* 实际转矩电流 */
      uint8_t motor_err     = rx_data[7];                                 /* 错误码 */

      /* 这些量先留着：要看波形时接到 OLED / 串口 / 调试器 live watch 上 */
      (void)rotor_angle;
      (void)rotor_speed;
      (void)real_current;
      (void)motor_err;

      /* 绿灯在闪 = CAN 通了、电调在回话；完全不闪 = 一帧反馈都没收到 */
      if (++rx_frame_cnt >= 500u)
      {
        rx_frame_cnt = 0;
        HAL_GPIO_TogglePin(LED_G_GPIO_Port, LED_G_Pin);
      }
    }
  }
}
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
#ifdef USE_FULL_ASSERT
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
