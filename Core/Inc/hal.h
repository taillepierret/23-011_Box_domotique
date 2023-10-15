/*
 * hal.h
 *
 *  Created on: Sep 18, 2023
 *      Author: teddy
 */

#ifndef INC_HAL_H_
#define INC_HAL_H_

#include "stdint.h"
#include "stdbool.h"


typedef enum {
	LED_VERTE_EN,
	LED_ROUGE_EN
}led_en;

typedef enum
{
	LED_SET_OK,
	LED_SET_ERROR,
	SPI_WRITE_ERROR_EN,
	SPI_WRITE_OK_EN,
	SPI_READ_ERROR_EN,
	SPI_READ_OK_EN
}HAL_ret_val_en;


HAL_ret_val_en HAL_setLed(led_en led_EN, bool state_B);
uint32_t HAL_millis_U32(void);
HAL_ret_val_en HAL_writeSpiValue_EN(uint8_t* value_U8A, uint16_t size_value_U16);
HAL_ret_val_en HAL_readSpiValue_EN(uint8_t reg_U8, uint8_t* read_value_U8A,uint16_t size_read_value_U16);
void HAL_setCE(bool state_B);
void HAL_setIRQ(bool state_B);
void HAL_delay_ms(uint32_t time_ms_U32);
void print_string(char* string);
void print_uint32(uint32_t number_U32);


#endif /* INC_HAL_H_ */
