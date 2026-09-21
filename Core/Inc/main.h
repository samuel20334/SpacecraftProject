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
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdbool.h>
#include <stdlib.h>
#include "MMC5983.h"
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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

// @brief	Wrapper to start PWM
// @param 	timer 	:	pointer to htim Timer object
// @param	channel : 	channel to be activated
// @retval 	None
void Start_PWM(TIM_HandleTypeDef *timer, uint32_t channel);

// @brief	Set the PWM duty cycle for a given channel
// @param	timer		:	pointer to htim Timer object
// @param	channel		:	channel to be activated
// @param	duty_cycle	: 	duty cycle to set PWM as %
// @retval 	None
void Set_Magnetorquer_PWM(TIM_HandleTypeDef *timer, uint32_t channel, uint8_t duty_cycle);


// @brief	Send a command to a magnetorquer with duty cycle and direction
// @param 	mag_num		: 	the number of the magnetorquer to send a command to (X_COIL, Y_COIL or Z_COIL)
// @param 	duty_cycle	:	duty cycle to set PWM as %
// @param	direction	: 	direction to drive magnetorquer in (FORWARD or REVERSE)
// @retval 	None
void Magnetorquer_Command(uint8_t mag_num, uint8_t duty_cycle, bool direction);

// @brief	Send commands to all three magnetorquers using signed duty cycles
// @param	xDuty	:	Signed duty cycle for x coil in % (-100 to 100)
// @param	yDuty	:	Signed duty cycle for y coil in % (-100 to 100)
// @param	zDuty	:	Signed duty cycle for z coil in % (-100 to 100)
// @retval 	None
void XYZ_Command(int8_t xDuty, int8_t yDuty, int8_t zDuty);

// @brief	Testing function to drive coils & send current + magnetometer readings
// @param	xDuty	:	Signed duty cycle for x coil in % (-100 to 100)
// @param	yDuty	:	Signed duty cycle for y coil in % (-100 to 100)
// @param	zDuty	:	Signed duty cycle for z coil in % (-100 to 100)
// @retval	None
void testCoil(int8_t xDuty, int8_t yDuty, int8_t zDuty);

// @brief	Run automated test routine to drive coils individually at max duty cycle in all directions
// @param 	None
// @retval	None
void runPowerProfile(void);

// @brief	Run automated test routine to drive coils with duty cycle in increments of 20%
// @param	None
// @retval	None
void runFunctionalTest(void);

// @brief	Initialise the MMC5983 magnetometer
// @param	magHandle	:	pointer to magnetometer object handle
// @retval	None
void MMC5983_Init(MMC5983_HW_InitTypeDef *magHandle);

// @brief	Calculate the current values based on ADC readings
// @param	raw		:	buffer of raw ADC values for current measurement
// @param	data	:	buffer to store calculated currents
// @retval	None
void Update_Current_Data(uint32_t *raw, int16_t *data);

// @brief	Send calculated current values over UART
// @param	huart			:	pointer to UART handle
// @param	current_data	:	buffer containing calculated current values
// @retval 	None
void Send_Current_Data(UART_HandleTypeDef *huart, int16_t *current_data);

// @brief	Send the magnetometer data over UART
// @param	huart			:	pointer to UART handle
// @param	MMC5983_Data	:	magnetometer data object containing updated magnetometer data
void Send_Magnetometer_Data(UART_HandleTypeDef *huart, MMC5983_Data_TypeDef *MMC5983_Data);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SPI2_CS_Pin GPIO_PIN_12
#define SPI2_CS_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
