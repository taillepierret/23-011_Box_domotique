/*
 * hal.c
 *
 *  Created on: Sep 18, 2023
 *      Author: teddy
 */

#include "hal.h"
#include "main.h"
#include <string.h>
#include <stdio.h>

extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart2;
extern RTC_HandleTypeDef hrtc;
static bool HAL_spiIsInit = false;
uint8_t rx_buffer_U8A[cSIZE_BUFFER_UART_2_RX_U16];

inline uint32_t HAL_millis_U32(void)
{
	return HAL_GetTick();
}

void HAL_spiInit(void* (SPI_init)(void))
{
	HAL_spiIsInit = true;
	SPI_init();
}

inline void HAL_setCE(bool state_B)
{
	HAL_GPIO_WritePin(GPIOB, CE_Pin, state_B);
}

inline void HAL_setIRQ(bool state_B)
{
	HAL_GPIO_WritePin(GPIOB, IRQ_Pin, state_B);
}

HAL_ret_val_en HAL_writeSpiValue_EN(uint8_t* value_U8A, uint16_t size_value_U16)
{
	HAL_StatusTypeDef ret_val;
	HAL_GPIO_WritePin(CSN_GPIO_Port, CSN_Pin, GPIO_PIN_RESET);
	ret_val = HAL_SPI_Transmit(&hspi1, value_U8A, size_value_U16, 100);
	HAL_GPIO_WritePin(CSN_GPIO_Port, CSN_Pin, GPIO_PIN_SET);
	if (ret_val != HAL_OK)
	{
		return HAL_SPI_WRITE_ERROR_EN;
	}
	else
	{
		return HAL_SPI_WRITE_OK_EN;
	}
}

HAL_ret_val_en HAL_readSpiValue_EN(uint8_t reg_U8, uint8_t* read_value_U8A,uint16_t size_read_value_U16)
{
	HAL_StatusTypeDef ret_val;
	HAL_GPIO_WritePin(CSN_GPIO_Port, CSN_Pin, GPIO_PIN_RESET);
	ret_val = HAL_SPI_Transmit(&hspi1, &reg_U8, 1, 100);
	if (ret_val != HAL_OK)
	{
		HAL_GPIO_WritePin(CSN_GPIO_Port, CSN_Pin, GPIO_PIN_SET);
		return HAL_SPI_READ_ERROR_EN;
	}
	ret_val = HAL_SPI_Receive(&hspi1, read_value_U8A, size_read_value_U16+1, 100);
	HAL_GPIO_WritePin(CSN_GPIO_Port, CSN_Pin, GPIO_PIN_SET);
	if (ret_val != HAL_OK)
	{
		return HAL_SPI_READ_ERROR_EN;
	}
	else
	{
		return HAL_SPI_READ_OK_EN;
	}
}

inline void HAL_delay_ms(uint32_t time_ms_U32)
{
	HAL_Delay(time_ms_U32);
}

inline void HAL_print_string(char* string)
{
	 HAL_UART_Transmit(&huart2, (uint8_t*)string, strlen(string), 1000);
}

/*void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	HAL_UART_Receive_DMA(&huart2, rx_buffer_U8A, cSIZE_BUFFER_UART_2_RX_U16);
}*/

inline void HAL_enableRxDmaUart2(void)
{
	HAL_UART_Receive_DMA(&huart2, rx_buffer_U8A, cSIZE_BUFFER_UART_2_RX_U16);
}

void HAL_getUart2Buffer(uint8_t rx_get_buffer_U8A[cSIZE_BUFFER_UART_2_RX_U16])
{
	for (uint8_t index_U8=0 ; index_U8<cSIZE_BUFFER_UART_2_RX_U16 ; index_U8++)
	{
		rx_get_buffer_U8A[index_U8] = rx_buffer_U8A[index_U8];
	}
}
void HAL_flushUart2Buffer(void)
{
	for (uint8_t index_U8=0 ; index_U8<cSIZE_BUFFER_UART_2_RX_U16 ; index_U8++)
	{
		rx_buffer_U8A[index_U8] = '\0';
	}
}

void HAL_GetTime(HAL_RTC_values_str* HAL_RTC_values_STR)
{ //Gerer la valeur de retour de HAL_RTC_getTime
	RTC_TimeTypeDef sTime_EN;
	HAL_RTC_GetTime(&hrtc, &sTime_EN, RTC_FORMAT_BIN);
	HAL_RTC_values_STR->hours_U8 = sTime_EN.Hours;
	HAL_RTC_values_STR->minutes_U8 = sTime_EN.Minutes;
	HAL_RTC_values_STR->seconds_U8 = sTime_EN.Seconds;
}

void HAL_InitDebugUart(void)
{
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
	Error_Handler();
	}
}

