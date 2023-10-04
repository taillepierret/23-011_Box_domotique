/*
  ***************************************************************************************************************
  ***************************************************************************************************************
  ***************************************************************************************************************

  File:		  NRF24L01.c
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


#include "main.h"
#include "NRF24L01.h"

static NRF_HAL_function_str NRF_HAL_function_local_STR;
static bool NRF_isInit_B = false;

extern SPI_HandleTypeDef hspi1;

void CS_Select (void)
{
	HAL_GPIO_WritePin(CSN_GPIO_Port, CSN_Pin, GPIO_PIN_RESET);
}

void CS_UnSelect (void)
{
	HAL_GPIO_WritePin(CSN_GPIO_Port, CSN_Pin, GPIO_PIN_SET);
}



// write a single byte to the particular register
void NRF_WriteReg_EN (uint8_t Reg, uint8_t Data)
{
	uint8_t buf[2];
	buf[0] = Reg|1<<5;
	buf[1] = Data;

	// Pull the CS Pin LOW to select the device
	CS_Select();

	HAL_writeSpiValue_EN(buf, 2);

	// Pull the CS HIGH to release the device
	CS_UnSelect();
}

//write multiple bytes starting from a particular register
void NRF_WriteReg_ENMulti (uint8_t Reg, uint8_t *data, int size)
{
	uint8_t buf[2];
	buf[0] = Reg|1<<5;
//	buf[1] = Data;

	// Pull the CS Pin LOW to select the device
	CS_Select();

	HAL_writeSpiValue_EN(buf, 1);
	HAL_writeSpiValue_EN(data, size);

	// Pull the CS HIGH to release the device
	CS_UnSelect();
}


static NRF_ret_val_en nrf24_ReadReg_EN(uint8_t register_REG, uint8_t* read_value_U8P)
{
	*read_value_U8P = 0;
	HAL_readSpiValue_EN((uint8_t)register_REG,read_value_U8P,1);
	return NRF_OK_EN;
}


/* Read multiple bytes from the register */
void nrf24_ReadReg_Multi_EN (uint8_t Reg, uint8_t *data, int size)
{
	// Pull the CS Pin LOW to select the device
	CS_Select();

	HAL_writeSpiValue_EN(&Reg, 1);
	HAL_SPI_Receive(&hspi1, data, size, 1000);

	// Pull the CS HIGH to release the device
	CS_UnSelect();
}


// send the command to the NRF
void nrfsendCmd_EN (uint8_t cmd)
{
	// Pull the CS Pin LOW to select the device
	CS_Select();

	HAL_writeSpiValue_EN(&cmd, 1);

	// Pull the CS HIGH to release the device
	CS_UnSelect();
}

void nrf24_reset(uint8_t REG)
{
	if (REG == STATUS)
	{
		NRF_WriteReg_EN(STATUS, 0x00);
	}

	else if (REG == FIFO_STATUS)
	{
		NRF_WriteReg_EN(FIFO_STATUS, 0x11);
	}

	else {
	NRF_WriteReg_EN(CONFIG, 0x08);
	NRF_WriteReg_EN(EN_AA, 0x3F);
	NRF_WriteReg_EN(EN_RXADDR, 0x03);
	NRF_WriteReg_EN(SETUP_AW, 0x03);
	NRF_WriteReg_EN(SETUP_RETR, 0x03);
	NRF_WriteReg_EN(RF_CH, 0x02);
	NRF_WriteReg_EN(RF_SETUP, 0x0E);
	NRF_WriteReg_EN(STATUS, 0x00);
	NRF_WriteReg_EN(OBSERVE_TX, 0x00);
	NRF_WriteReg_EN(CD, 0x00);
	uint8_t rx_addr_p0_def[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
	NRF_WriteReg_ENMulti(RX_ADDR_P0, rx_addr_p0_def, 5);
	uint8_t rx_addr_p1_def[5] = {0xC2, 0xC2, 0xC2, 0xC2, 0xC2};
	NRF_WriteReg_ENMulti(RX_ADDR_P1, rx_addr_p1_def, 5);
	NRF_WriteReg_EN(RX_ADDR_P2, 0xC3);
	NRF_WriteReg_EN(RX_ADDR_P3, 0xC4);
	NRF_WriteReg_EN(RX_ADDR_P4, 0xC5);
	NRF_WriteReg_EN(RX_ADDR_P5, 0xC6);
	uint8_t tx_addr_def[5] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};
	NRF_WriteReg_ENMulti(TX_ADDR, tx_addr_def, 5);
	NRF_WriteReg_EN(RX_PW_P0, 0);
	NRF_WriteReg_EN(RX_PW_P1, 0);
	NRF_WriteReg_EN(RX_PW_P2, 0);
	NRF_WriteReg_EN(RX_PW_P3, 0);
	NRF_WriteReg_EN(RX_PW_P4, 0);
	NRF_WriteReg_EN(RX_PW_P5, 0);
	NRF_WriteReg_EN(FIFO_STATUS, 0x11);
	NRF_WriteReg_EN(DYNPD, 0);
	NRF_WriteReg_EN(FEATURE, 0);
	}
}




void NRF24_Init_EN(NRF_HAL_function_str NRF_HAL_function_STR)
{
	NRF_HAL_function_local_STR.readSpiValue_EN_PF = NRF_HAL_function_STR.readSpiValue_EN_PF;
	NRF_HAL_function_local_STR.setCe_PF = NRF_HAL_function_STR.setCe_PF;
	NRF_HAL_function_local_STR.setIrq_PF = NRF_HAL_function_STR.setIrq_PF;
	NRF_HAL_function_local_STR.writeSpiValue_EN_PF = NRF_HAL_function_STR.writeSpiValue_EN_PF;

	// disable the chip before configuring the device
	NRF_HAL_function_local_STR.setCe_PF(false);
	CS_UnSelect();


	// reset everything
	nrf24_reset (0);

	NRF_WriteReg_EN(CONFIG, 0);  // will be configured later

	NRF_WriteReg_EN(EN_AA, 0);  // No Auto ACK

	NRF_WriteReg_EN (EN_RXADDR, 0);  // Not Enabling any data pipe right now

	NRF_WriteReg_EN (SETUP_AW, 0x03);  // 5 Bytes for the TX/RX address

	NRF_WriteReg_EN (SETUP_RETR, 0);   // No retransmission

	NRF_WriteReg_EN (RF_CH, 0);  // will be setup during Tx or RX

	NRF_WriteReg_EN (RF_SETUP, 0x0E);   // Power= 0db, data rate = 2Mbps

	// Enable the chip after configuring the device
	NRF_HAL_function_local_STR.setCe_PF(true);
	CS_Select();
	NRF_isInit_B = true;

}


// set up the Tx mode

void NRF24_TxMode (uint8_t *Address, uint8_t channel)
{
	// disable the chip before configuring the device
	NRF_HAL_function_local_STR.setCe_PF(false);
	CS_UnSelect();

	NRF_WriteReg_EN (RF_CH, channel);  // select the channel

	NRF_WriteReg_ENMulti(TX_ADDR, Address, 5);  // Write the TX address


	// power up the device
	uint8_t config = 0;
	nrf24_ReadReg_EN(CONFIG,&config);
	config = config | (1<<1);   // write 1 in the PWR_UP bit
	//config = config & (0xF2);    // write 0 in the PRIM_RX, and 1 in the PWR_UP, and all other bits are masked
	NRF_WriteReg_EN (CONFIG, config);

	// Enable the chip after configuring the device
	NRF_HAL_function_local_STR.setCe_PF(true);
}


// transmit the data

uint8_t NRF24_Transmit (uint8_t *data)
{
	uint8_t cmdtosend = 0;
	uint8_t fifostatus = 0;

	// select the device
	CS_Select();

	// payload command
	cmdtosend = W_TX_PAYLOAD;
	HAL_writeSpiValue_EN(&cmdtosend, 1);

	// send the payload
	HAL_writeSpiValue_EN(data, 32);

	// Unselect the device
	CS_UnSelect();

	HAL_delay_ms(1);

	nrf24_ReadReg_EN(FIFO_STATUS, &fifostatus);

	// check the fourth bit of FIFO_STATUS to know if the TX fifo is empty
	if ((fifostatus&(1<<4)) && (!(fifostatus&(1<<3))))
	{
		cmdtosend = FLUSH_TX;
		nrfsendCmd_EN(cmdtosend);
		return 1;
	}

	return 0;
}


void NRF24_RxMode (uint8_t *Address, uint8_t channel)
{
	// disable the chip before configuring the device
	NRF_HAL_function_local_STR.setCe_PF(false);

	nrf24_reset (STATUS);

	NRF_WriteReg_EN (RF_CH, channel);  // select the channel

	// select data pipe 2
	uint8_t en_rxaddr = 0;
	nrf24_ReadReg_EN(EN_RXADDR,&en_rxaddr);
	en_rxaddr = en_rxaddr | (1<<1);
	NRF_WriteReg_EN (EN_RXADDR, en_rxaddr);

	/* We must write the address for Data Pipe 1, if we want to use any pipe from 2 to 5
	 * The Address from DATA Pipe 2 to Data Pipe 5 differs only in the LSB
	 * Their 4 MSB Bytes will still be same as Data Pipe 1
	 *
	 * For Eg->
	 * Pipe 1 ADDR = 0xAABBCCDD11
	 * Pipe 2 ADDR = 0xAABBCCDD22
	 * Pipe 3 ADDR = 0xAABBCCDD33
	 *
	 */
	NRF_WriteReg_ENMulti(RX_ADDR_P1, Address, 5);  // Write the Pipe1 address
	//NRF_WriteReg_EN(RX_ADDR_P2, 0xEE);  // Write the Pipe2 LSB address

	NRF_WriteReg_EN (RX_PW_P1, 32);   // 32 bit payload size for pipe 2


	// power up the device in Rx mode
	uint8_t config = 0;
	nrf24_ReadReg_EN(CONFIG,&config);
	config = config | (1<<1) | (1<<0);
	NRF_WriteReg_EN (CONFIG, config);

	// Enable the chip after configuring the device
	NRF_HAL_function_local_STR.setCe_PF(true);
}


NRF_ret_val_en isDataAvailable_EN(int pipenum)
{
	uint8_t status = 0;
	nrf24_ReadReg_EN(STATUS,&status);

	if ((status&(1<<6))&&(status&(pipenum<<1)))
	{

		NRF_WriteReg_EN(STATUS, (1<<6));

		return NRF_DATA_AVAILABLE_EN;
	}

	return NRF_DATA_NOT_AVAILABLE_EN;
}


void NRF24_Receive (uint8_t *data)
{
	uint8_t cmdtosend = 0;

	// payload command
	HAL_readSpiValue_EN(R_RX_PAYLOAD,data,32);

	HAL_delay_ms(1);

	cmdtosend = FLUSH_RX;
	nrfsendCmd_EN(cmdtosend);
}



// Read all the Register data
void NRF24_ReadAll (uint8_t *data)
{
	for (int i=0; i<10; i++)
	{
		nrf24_ReadReg_EN(i,(uint8_t*)(data+i));
	}

	nrf24_ReadReg_Multi_EN(RX_ADDR_P0, (data+10), 5);

	nrf24_ReadReg_Multi_EN(RX_ADDR_P1, (data+15), 5);

	nrf24_ReadReg_EN(RX_ADDR_P2,(uint8_t*)(data+20));
	nrf24_ReadReg_EN(RX_ADDR_P3,(uint8_t*)(data+21));
	nrf24_ReadReg_EN(RX_ADDR_P4,(uint8_t*)(data+22));
	nrf24_ReadReg_EN(RX_ADDR_P5,(uint8_t*)(data+23));

	nrf24_ReadReg_Multi_EN(RX_ADDR_P0, (data+24), 5);

	for (int i=29; i<38; i++)
	{
		nrf24_ReadReg_EN(i-12,(uint8_t*)(data+i));
	}

}









