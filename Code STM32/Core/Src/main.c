/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#define DS3231_ADDRESS 0xD0
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim14;

/* USER CODE BEGIN PV */
typedef struct {
	int hour;
	int minute;
	int month;
	int dom;
	int dow;
} timeData_t;

typedef struct {
	uint8_t Digit1;
	uint8_t Digit2;
	uint8_t Digit3;
	uint8_t Digit4;
} digit_t;

typedef enum {
	hour_n_minute,
	day_of_week,
	date_n_month,
} timeComponent_t;

timeComponent_t timeComponent = 0;
timeData_t timeNow;
digit_t timeDigit;
uint32_t sweepTime = 0;
uint8_t digit = 1;
uint8_t flag = 1;
int count = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);
static void MX_TIM14_Init(void);
/* USER CODE BEGIN PFP */
uint8_t decToBcd(int val);
int bcdToDec(uint8_t val);
void Set_Time (uint8_t sec, uint8_t min, uint8_t hour, uint8_t dow, uint8_t dom, uint8_t month, uint8_t year);
void Get_Time (void);
void Display_Time(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == htim3.Instance) flag = 1;
  if (htim->Instance == htim14.Instance){
	  if (timeComponent == 2) timeComponent = 0;
	  else timeComponent++;
  }
}
// Convert normal decimal numbers to binary coded decimal
uint8_t decToBcd(int val)
{
  return (uint8_t)( (val/10*16) + (val%10) );
}
// Convert binary coded decimal to normal decimal numbers
int bcdToDec(uint8_t val)
{
  return (int)( (val/16*10) + (val%16) );
}
/* function to set time */

void Set_Time (uint8_t sec, uint8_t min, uint8_t hour, uint8_t dow, uint8_t dom, uint8_t month, uint8_t year)
{
	uint8_t set_time[7];
	set_time[0] = decToBcd(sec);
	set_time[1] = decToBcd(min);
	set_time[2] = decToBcd(hour);
	set_time[3] = decToBcd(dow);
	set_time[4] = decToBcd(dom);
	set_time[5] = decToBcd(month);
	set_time[6] = decToBcd(year);

	HAL_I2C_Mem_Write(&hi2c1, DS3231_ADDRESS, 0x00, 1, set_time, 7, 1000);
}
//function to get time
void Get_Time (void)
{
	uint8_t get_time[6];
	HAL_I2C_Mem_Read(&hi2c1, DS3231_ADDRESS, 0x01, 1, get_time, 6, 1000);
	timeNow.minute = bcdToDec(get_time[0]);
	timeNow.hour = bcdToDec(get_time[1]);
	timeNow.dow = bcdToDec(get_time[2]);
	timeNow.dom = bcdToDec(get_time[3]);
	timeNow.month = bcdToDec(get_time[4]);
	switch (timeComponent) {
		case hour_n_minute: {
			timeDigit.Digit1 = decToBcd((timeNow.hour/10));
			timeDigit.Digit2 = decToBcd((timeNow.hour%10));
			timeDigit.Digit3 = decToBcd((timeNow.minute/10));
			timeDigit.Digit4 = decToBcd((timeNow.minute%10));
			break;
		}
		case date_n_month: {
			timeDigit.Digit1 = decToBcd((timeNow.dom/10));
			timeDigit.Digit2 = decToBcd((timeNow.dom%10));
			timeDigit.Digit3 = decToBcd((timeNow.month/10));
			timeDigit.Digit4 = decToBcd((timeNow.month%10));
			break;
		}
		case day_of_week: {
			timeDigit.Digit1 = 0b1111;
			timeDigit.Digit2 = 0b1111;
			timeDigit.Digit3 = 0b1110;
			timeDigit.Digit4 = decToBcd(timeNow.dow);
			break;
		}
		default: break;
	}
}

void Display_Time(void)
{
	Get_Time();
	switch(digit){
			case 1:
				if(flag){
					HAL_GPIO_WritePin(DIGIT_1_GPIO_Port, DIGIT_1_Pin, 1);
					HAL_GPIO_WritePin(DIGIT_2_GPIO_Port, DIGIT_2_Pin, 0);
					HAL_GPIO_WritePin(DIGIT_3_GPIO_Port, DIGIT_3_Pin, 0);
					HAL_GPIO_WritePin(DIGIT_4_GPIO_Port, DIGIT_4_Pin, 0);
					HAL_GPIO_WritePin(BIT_A3_GPIO_Port, BIT_A3_Pin, timeDigit.Digit1 & 0b0001);
					HAL_GPIO_WritePin(BIT_A0_GPIO_Port, BIT_A0_Pin, timeDigit.Digit1 & 0b0010);
					HAL_GPIO_WritePin(BIT_A1_GPIO_Port, BIT_A1_Pin, timeDigit.Digit1 & 0b0100);
					HAL_GPIO_WritePin(BIT_A2_GPIO_Port, BIT_A2_Pin, timeDigit.Digit1 & 0b1000);
					digit = 2;
					flag = 0;
				}
				break;
			case 2:
				if(flag){
					HAL_GPIO_WritePin(DIGIT_2_GPIO_Port, DIGIT_2_Pin, 1);
					HAL_GPIO_WritePin(DIGIT_1_GPIO_Port, DIGIT_1_Pin, 0);
					HAL_GPIO_WritePin(DIGIT_3_GPIO_Port, DIGIT_3_Pin, 0);
					HAL_GPIO_WritePin(DIGIT_4_GPIO_Port, DIGIT_4_Pin, 0);
					HAL_GPIO_WritePin(BIT_A3_GPIO_Port, BIT_A3_Pin, timeDigit.Digit2 & 0b0001);
					HAL_GPIO_WritePin(BIT_A0_GPIO_Port, BIT_A0_Pin, timeDigit.Digit2 & 0b0010);
					HAL_GPIO_WritePin(BIT_A1_GPIO_Port, BIT_A1_Pin, timeDigit.Digit2 & 0b0100);
					HAL_GPIO_WritePin(BIT_A2_GPIO_Port, BIT_A2_Pin, timeDigit.Digit2 & 0b1000);
					digit = 3;
					flag = 0;
				}
				break;
			case 3:
				if(flag){
					HAL_GPIO_WritePin(DIGIT_3_GPIO_Port, DIGIT_3_Pin, 1);
					HAL_GPIO_WritePin(DIGIT_1_GPIO_Port, DIGIT_1_Pin, 0);
					HAL_GPIO_WritePin(DIGIT_2_GPIO_Port, DIGIT_2_Pin, 0);
					HAL_GPIO_WritePin(DIGIT_4_GPIO_Port, DIGIT_4_Pin, 0);
					HAL_GPIO_WritePin(BIT_A3_GPIO_Port, BIT_A3_Pin, timeDigit.Digit3 & 0b0001);
					HAL_GPIO_WritePin(BIT_A0_GPIO_Port, BIT_A0_Pin, timeDigit.Digit3 & 0b0010);
					HAL_GPIO_WritePin(BIT_A1_GPIO_Port, BIT_A1_Pin, timeDigit.Digit3 & 0b0100);
					HAL_GPIO_WritePin(BIT_A2_GPIO_Port, BIT_A2_Pin, timeDigit.Digit3 & 0b1000);
					digit = 4;
					flag = 0;
				}
				break;
			case 4:
				if(flag){
					HAL_GPIO_WritePin(DIGIT_4_GPIO_Port, DIGIT_4_Pin, 1);
					HAL_GPIO_WritePin(DIGIT_1_GPIO_Port, DIGIT_1_Pin, 0);
					HAL_GPIO_WritePin(DIGIT_2_GPIO_Port, DIGIT_2_Pin, 0);
					HAL_GPIO_WritePin(DIGIT_3_GPIO_Port, DIGIT_3_Pin, 0);
					HAL_GPIO_WritePin(BIT_A3_GPIO_Port, BIT_A3_Pin, timeDigit.Digit4 & 0b0001);
					HAL_GPIO_WritePin(BIT_A0_GPIO_Port, BIT_A0_Pin, timeDigit.Digit4 & 0b0010);
					HAL_GPIO_WritePin(BIT_A1_GPIO_Port, BIT_A1_Pin, timeDigit.Digit4 & 0b0100);
					HAL_GPIO_WritePin(BIT_A2_GPIO_Port, BIT_A2_Pin, timeDigit.Digit4 & 0b1000);
					digit = 1;
					flag = 0;
				}
				break;
	}
}
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
  MX_I2C1_Init();
  MX_TIM3_Init();
  MX_TIM14_Init();
  /* USER CODE BEGIN 2 */
//  Set_Time(30, 35, 16, 7, 26, 11, 22);
  HAL_TIM_Base_Start_IT(&htim3);
  HAL_TIM_Base_Start_IT(&htim14);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  Display_Time();
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_I2C1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
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
  hi2c1.Init.Timing = 0x2000090E;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 50;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief TIM14 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM14_Init(void)
{

  /* USER CODE BEGIN TIM14_Init 0 */

  /* USER CODE END TIM14_Init 0 */

  /* USER CODE BEGIN TIM14_Init 1 */

  /* USER CODE END TIM14_Init 1 */
  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 4000;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 10000;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM14_Init 2 */

  /* USER CODE END TIM14_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, BIT_A0_Pin|BIT_A1_Pin|BIT_A2_Pin|BIT_A3_Pin
                          |DIGIT_1_Pin|DIGIT_2_Pin|DIGIT_3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, DIGIT_4_Pin|LED_LIFE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : BIT_A0_Pin BIT_A1_Pin BIT_A2_Pin BIT_A3_Pin
                           DIGIT_1_Pin DIGIT_2_Pin DIGIT_3_Pin */
  GPIO_InitStruct.Pin = BIT_A0_Pin|BIT_A1_Pin|BIT_A2_Pin|BIT_A3_Pin
                          |DIGIT_1_Pin|DIGIT_2_Pin|DIGIT_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : DIGIT_4_Pin */
  GPIO_InitStruct.Pin = DIGIT_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DIGIT_4_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LED_LIFE_Pin */
  GPIO_InitStruct.Pin = LED_LIFE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_LIFE_GPIO_Port, &GPIO_InitStruct);

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
