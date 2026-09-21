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
#define MAX_OCR 65535

// TIMER 1
#define COILA1_CHANNEL TIM_CHANNEL_1
#define COILA2_CHANNEL TIM_CHANNEL_2
#define COILB1_CHANNEL TIM_CHANNEL_3
#define COILB2_CHANNEL TIM_CHANNEL_4

// TIMER 3
#define COILC1_CHANNEL TIM_CHANNEL_1
#define COILC2_CHANNEL TIM_CHANNEL_2

// Macros for coils
#define X_COIL 0
#define Y_COIL 1
#define Z_COIL 2

// Macros for direction
#define FORWARD true
#define REVERSE false

// Number of ADC Channels
#define TOTAL_ADC_CHANNELS 3

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart4;
UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
MMC5983_HW_InitTypeDef MMC5983_Handle = {
	    .SPIhandler = &hspi2,
	    .CS_GPIOport = GPIOB,
	    .CS_GPIOpin = GPIO_PIN_12,
	    .SPI_Timeout = 100
	};

MMC5983_Data_TypeDef mag_data;

uint32_t adc_data[TOTAL_ADC_CHANNELS];
int16_t current_data[TOTAL_ADC_CHANNELS];

static uint8_t uartRxData[8] = {0};

// flags
bool coilCommandFlag = false;
bool currentReadingFlag = false;
bool magReadingFlag = false;
bool powerProfileFlag = false;
bool testRoutineFlag = false;

// variables for storing sent params
int8_t xDutyCycle = 0;
int8_t yDutyCycle = 0;
int8_t zDutyCycle = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC1_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM3_Init(void);
static void MX_UART4_Init(void);
static void MX_USART1_UART_Init(void);
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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_TIM3_Init();
  MX_UART4_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

  if (HAL_UART_Receive_IT(&huart4, uartRxData, 4) != HAL_OK)
  {
	  Error_Handler();
  }
  // Start ADCs
  HAL_ADC_Start_DMA(&hadc1, adc_data, 3);

  // Initialise magnetometer
  MMC5983_Init(&MMC5983_Handle);

  // Start PWM on all channels
  Start_PWM(&htim1, COILA1_CHANNEL);
  Start_PWM(&htim1, COILA2_CHANNEL);
  Start_PWM(&htim1, COILB1_CHANNEL);
  Start_PWM(&htim1, COILB2_CHANNEL);
  Start_PWM(&htim3, COILC1_CHANNEL);
  Start_PWM(&htim3, COILC2_CHANNEL);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	if (coilCommandFlag) {
		if (abs(xDutyCycle) > 100) xDutyCycle = 0;
		if (abs(yDutyCycle) > 100) xDutyCycle = 0;
		if (abs(zDutyCycle) > 100) xDutyCycle = 0;

		XYZ_Command(xDutyCycle, yDutyCycle, zDutyCycle);

		coilCommandFlag = false;
	}

	if (currentReadingFlag) {
		Update_Current_Data(adc_data, current_data);
		Send_Current_Data(&huart4, current_data);

		currentReadingFlag = false;
	}

	if (magReadingFlag) {
		MMC5983_Data_Read(&MMC5983_Handle, &mag_data);
		Send_Magnetometer_Data(&huart4, &mag_data);

		magReadingFlag = false;
	}

	if (powerProfileFlag) {
		runPowerProfile();

		powerProfileFlag = false;
	}

	if (testRoutineFlag) {
		runFunctionalTest();

		testRoutineFlag = false;
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 3;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = ADC_REGULAR_RANK_3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_SLAVE;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_4BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 7;
  hspi2.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

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
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.BreakFilter = 0;
  sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
  sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
  sBreakDeadTimeConfig.Break2Filter = 0;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

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
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 115200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);

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
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9|GPIO_PIN_10, GPIO_PIN_RESET);

  /*Configure GPIO pin : SPI2_CS_Pin */
  GPIO_InitStruct.Pin = SPI2_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SPI2_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PC9 PC10 */
  GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PC11 PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
// Callback for UART hardware interrupt
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{

	switch (uartRxData[0]) {
	case 0x00:
		// Send coil command
		coilCommandFlag = true;
		xDutyCycle = uartRxData[1];
		yDutyCycle = uartRxData[2];
		zDutyCycle = uartRxData[3];
		break;

	case 0x01:
		// Read current data
		currentReadingFlag = true;
		break;

	case 0x02:
		// Read magnetometer data
		magReadingFlag = true;
		break;

	case 0x03:
		// run power profile test
		powerProfileFlag = true;
		break;

	case 0x04:
		// run functional test
		testRoutineFlag = true;
		break;
	}


    // Receive next
	if (HAL_UART_Receive_IT(huart, uartRxData, 4) != HAL_OK)
	{
	    Error_Handler();
	}
}

void Start_PWM(TIM_HandleTypeDef *timer, uint32_t channel) {
	if (HAL_TIM_PWM_Start(timer, channel) != HAL_OK) {
		Error_Handler();
	}
}

void Set_Magnetorquer_PWM(TIM_HandleTypeDef *timer, uint32_t channel, uint8_t duty_cycle) {
	uint32_t comparVal = ((float)duty_cycle/100)*MAX_OCR;	// calculate compare value from duty cycle
	__HAL_TIM_SET_COMPARE(timer, channel, comparVal);		// set the compare value
}

void Magnetorquer_Command(uint8_t mag_num, uint8_t duty_cycle, bool direction) {
	uint32_t channel1;
	uint32_t channel2;
	TIM_HandleTypeDef timer;

	switch(mag_num) {
	case 0:		// x coil
		timer = htim1;
		channel1 = COILA1_CHANNEL;
		channel2 = COILA2_CHANNEL;
		break;
	case 1:		// y coil
		timer = htim1;
		channel1 = COILB1_CHANNEL;
		channel2 = COILB2_CHANNEL;
		break;
	case 2:		// z coil
		timer = htim3;
		channel1 = COILC1_CHANNEL;
		channel2 = COILC2_CHANNEL;
		break;
	default:	// pick the x coil by default
		timer = htim1;
		channel1 = COILA1_CHANNEL;
		channel2 = COILA2_CHANNEL;
		break;
	}

	if (direction == FORWARD) {
		Set_Magnetorquer_PWM(&timer, channel1, duty_cycle);	// set PWM
		Set_Magnetorquer_PWM(&timer, channel2, 0);			// drive coil forwards
	} else {
		Set_Magnetorquer_PWM(&timer, channel1, 0);			// drive coil backwards
		Set_Magnetorquer_PWM(&timer, channel2, duty_cycle);	// set PWM
	}


}

void XYZ_Command(int8_t xDuty, int8_t yDuty, int8_t zDuty) {
	uint8_t xDutyCycle = abs(xDuty);	// find magnitude of duty cycles
	uint8_t yDutyCycle = abs(xDuty);
	uint8_t zDutyCycle = abs(xDuty);

	bool xDirection;
	bool yDirection;
	bool zDirection;

	xDirection = (xDuty > 0) ? FORWARD : REVERSE;	// set directions according to sign of duty cycle input
	yDirection = (yDuty > 0) ? FORWARD : REVERSE;
	zDirection = (zDuty > 0) ? FORWARD : REVERSE;

	Magnetorquer_Command(X_COIL, xDutyCycle, xDirection);	// send magnetorquer commands
	Magnetorquer_Command(Y_COIL, yDutyCycle, yDirection);
	Magnetorquer_Command(Z_COIL, zDutyCycle, zDirection);
}

void testCoil(int8_t xDuty, int8_t yDuty, int8_t zDuty) {
	XYZ_Command(xDuty, yDuty, zDuty);					// drive coils
	Update_Current_Data(adc_data, current_data);		// update current data
	MMC5983_Data_Read(&MMC5983_Handle, &mag_data);		// read magnetometer data
	Send_Current_Data(&huart4, current_data);			// send current data
	Send_Magnetometer_Data(&huart4, &mag_data);			// send magnetometer data
}

void runPowerProfile(void) {
	testCoil(100, 0, 0);	// drive X coil fully forward
	HAL_Delay(20000);
	testCoil(-100, 0, 0);	// drive X coil fully reverse
	HAL_Delay(20000);
	testCoil(0, 0, 0);		// de-energise X coil

	testCoil(0, 100, 0);	// drive Y coil fully forward
	HAL_Delay(20000);
	testCoil(0, -100, 0);	// drive Y coil fully reverse
	HAL_Delay(20000);
	testCoil(0, 0, 0);		// de-energise Y coil

	testCoil(0, 0, 100);	// drive Z coil fully forward
	HAL_Delay(20000);
	testCoil(0, 0, -100);	// drive Z coil fully reverse
	HAL_Delay(20000);
	testCoil(0, 0, 0);		// de-energise Z coil
}

void runFunctionalTest(void) {
	for (int i = 0; i <= 100; i += 10) {	// set x coil duty cycle in increments of 10%
		testCoil(i, 0, 0);
		HAL_Delay(20000);

		testCoil(-i, 0, 0);
		HAL_Delay(20000);
	}

	testCoil(0, 0, 0);

	for (int i = 0; i <= 100; i += 10) {	// set y coil duty cycle in increments of 10%
		testCoil(0, i, 0);
		HAL_Delay(20000);

		testCoil(0, -i, 0);
		HAL_Delay(20000);
	}

	testCoil(0, 0, 0);

	for (int i = 0; i <= 100; i += 10) {	// set z coil duty cycle in increments of 10%
		testCoil(0, 0, i);
		HAL_Delay(20000);

		testCoil(0, 0, -i);
		HAL_Delay(20000);
	}

	testCoil(0, 0, 0);
}

void MMC5983_Init(MMC5983_HW_InitTypeDef *magHandle) {
	if (MMC5983_ID_Verify(magHandle) != MMC_NO_ERROR) {
	    Error_Handler();
	}

	MMC5983_SW_Reset(magHandle);
	MMC5983_Set_Bandwidth(magHandle, MMC_BW_0400);
	MMC5983_Set_Operation_Rate(magHandle, MMC_SET_0500);
	MMC5983_Enable_Auto_Set_Reset(magHandle);
	MMC5983_Set_Output_DataRate(magHandle, MMC_ODR_0100);
	MMC5983_Set_Continuous_Measurement(magHandle);
}

void Update_Current_Data(uint32_t *raw, int16_t *data) {
	for (int ch = 0; ch < TOTAL_ADC_CHANNELS; ch++) {
		float raw_voltage = 3.3f*(float)raw[ch]/4095;	// convert adc value to voltage
		float shunt_voltage = raw_voltage / 20; 		// amplifier has gain of 20 V/V
		float current = shunt_voltage / 0.001;			// calculate current through the shunt
		data[ch] = (int16_t)(current * 1000);			// cast to int to send
	}
}

void Send_Current_Data(UART_HandleTypeDef *huart, int16_t *current_data) {
	uint8_t data[TOTAL_ADC_CHANNELS*2];

	for (int ch = 0; ch < TOTAL_ADC_CHANNELS; ch++) {
		uint16_t u_val = (uint16_t)current_data[ch];

		data[2*ch] = (u_val >> 8) & 0xFF;	// big endian encoding
		data[2*ch + 1] = u_val & 0xFF;
	}

	HAL_UART_Transmit(huart, data, TOTAL_ADC_CHANNELS*2, 1000);
}

void Send_Magnetometer_Data(UART_HandleTypeDef *huart, MMC5983_Data_TypeDef *MMC5983_Data) {
	uint8_t data[6];

	uint16_t uval_x = (uint16_t)MMC5983_Data->axes.DX;
	uint16_t uval_y = (uint16_t)MMC5983_Data->axes.DY;
	uint16_t uval_z = (uint16_t)MMC5983_Data->axes.DZ;

	data[0] = (uval_x >> 8) & 0xFF;
	data[1] = uval_x & 0xFF;
	data[2] = (uval_y >> 8) & 0xFF;
	data[3] = uval_y & 0xFF;
	data[4] = (uval_z >> 8) & 0xFF;
	data[5] = uval_z & 0xFF;

	HAL_UART_Transmit(huart, data, 6, 1000);
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
