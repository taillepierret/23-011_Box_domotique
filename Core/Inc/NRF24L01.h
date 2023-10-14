/*
  ***************************************************************************************************************
  ***************************************************************************************************************
  ***************************************************************************************************************

  File:		  NRF24L01.h
  Author:     ControllersTech.com
  Updated:    30th APRIL 2021

  ***************************************************************************************************************
  Copyright (C) 2017 ControllersTech.com

  This is a free software under the GNU license, you can redistribute it and/or modify it under the terms
  of the GNU General Public License version 3 as published by the Free Software Foundation.
  This software library is shared with public for educational purposes, without WARRANTY and Author is not liable for any damages caused directly
  or indirectly by this software, read more about this on the GNU General Public License.

  ***************************************************************************************************************
*/

#ifndef INC_NRF24L01_H_
#define INC_NRF24L01_H_

#include "stdint.h"
#include "hal.h"

#define cSIZE_BUFFER_TX_MAX_U8 32

typedef enum{
	CONFIG_REG = 0x00,
	EN_AA_REG = 0x01,
	EN_RXADDR_REG = 0x02,
	SETUP_AW_REG = 0x03,
	SETUP_RETR_REG = 0x04,
	RF_CH_REG = 0x05,
	RF_SETUP_REG = 0x06,
	STATUS_REG = 0x07,
	OBSERVE_TX_REG = 0x08,
	CD_REG = 0x09,
	RX_ADDR_P0_REG = 0x0A,
	RX_ADDR_P1_REG = 0x0B,
	RX_ADDR_P2_REG = 0x0B,
	RX_ADDR_P3_REG = 0x0D,
	RX_ADDR_P4_REG = 0x0E,
	RX_ADDR_P5_REG = 0x0F,
	TX_ADDR_REG = 0x10,
	RX_PW_P0_REG = 0x11,
	RX_PW_P1_REG = 0x12,
	RX_PW_P2_REG = 0x13,
	RX_PW_P3_REG = 0x14,
	RX_PW_P4_REG = 0x15,
	RX_PW_P5_REG = 0x16,
	FIFO_STATUS_REG = 0x17,
	DYNPD_REG = 0x1C,
	FEATURE_REG	= 0x1D,
	R_REGISTER_REG = 0x00,
	W_REGISTER_REG = 0x20,
	REGISTER_MASK_REG = 0x1F,
	ACTIVATE_REG = 0x50,
	R_RX_PL_WID_REG = 0x60,
	R_RX_PAYLOAD_REG = 0x61,
	W_TX_PAYLOAD_REG = 0xA0,
	W_ACK_PAYLOAD_REG = 0xA8,
	FLUSH_TX_REG = 0xE1,
	FLUSH_RX_REG = 0xE2,
	REUSE_TX_PL_REG = 0xE3,
	NOP_REG = 0xFF
}NRF_register_REG;

typedef enum{
	NRF_NOT_INIT_EN,
	NRF_SPI_ERROR_EN,
	NRF_SIZE_BUFFER_TX_TOO_LARGE_EN,
	NRF_OK_EN,
	NRF_DATA_AVAILABLE_EN,
	NRF_DATA_NOT_AVAILABLE_EN
}NRF_ret_val_en;

typedef struct{
	void (*setCe_PF)(bool state_B);
	void (*setIrq_PF)(bool state_B);
	HAL_ret_val_en (*writeSpiValue_EN_PF)(uint8_t* write_value_U8, uint16_t size_value_U16);
	HAL_ret_val_en (*readSpiValue_EN_PF)(uint8_t reg_U8, uint8_t* read_value_U8A,uint16_t size_read_value_U16);
}NRF_HAL_function_str;


NRF_ret_val_en NRF24_Init_EN(NRF_HAL_function_str NRF_HAL_function_STR);
NRF_ret_val_en NRF24_TxMode_EN (uint8_t *Address, uint8_t channel);
NRF_ret_val_en NRF24_Transmit_EN(uint8_t *data_U8A, uint8_t size_data_U8);
NRF_ret_val_en NRF24_RxMode_EN (uint8_t *Address, uint8_t channel);
NRF_ret_val_en isDataAvailable_EN (uint8_t pipenum_U8);
NRF_ret_val_en NRF24_Receive_EN (uint8_t *data_U8A);
void NRF24_ReadAll (uint8_t *data);


#endif /* INC_NRF24L01_H_ */
