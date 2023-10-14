/*
 * Application.c
 *
 *  Created on: Sep 30, 2023
 *      Author: teddy
 */

#include "nrf24l01.h"
#include "hal.h"
#include "main.h"

/*const static NRF_HAL_function_str NRF_HAL_function_local_STR =
{
	.readSpiValue_EN_PF = HAL_readSpiValue_EN,
	.setCe_PF = HAL_setCE,
	.setIrq_PF = HAL_setIRQ,
	.writeSpiValue_EN_PF = HAL_writeSpiValue_EN
};

void APP_testRx(void)
{
	uint8_t RxAddress[] = {0x00,0xDD,0xCC,0xBB,0xAA};
	uint8_t RxData[32];
	bool flag_rx_data = false;
	NRF_ret_val_en NRF_ret_val_EN;
	uint8_t data[50];

	NRF_ret_val_EN = NRF24_Init_EN(NRF_HAL_function_local_STR);
	if (NRF_ret_val_EN != NRF_OK_EN)
	{
		while(1);
	}

	NRF_ret_val_EN = NRF24_RxMode_EN(RxAddress, 10);
	if (NRF_ret_val_EN != NRF_OK_EN)
	{
		while(1);
	}

	NRF_ret_val_EN = NRF24_ReadAll_EN(data);
	if (NRF_ret_val_EN != NRF_OK_EN)
	{
		while(1);
	}

	while(1)
	{
		NRF_ret_val_EN = isDataAvailable_EN(2,&flag_rx_data);
		if (NRF_ret_val_EN != NRF_OK_EN)
		{
			while(1);
		}
		if (flag_rx_data == true && NRF_ret_val_EN == NRF_OK_EN)
		{
			NRF24_Receive_EN(RxData);
			//HAL_UART_Transmit(&huart2, RxData, strlen((char *)RxData), 1000);
		}
	}
}


void APP_testTx(void)
{
	NRF_ret_val_en NRF_ret_val_EN;
	uint8_t data[50];
	uint8_t TxAddress[] = {0xEE,0xDD,0xCC,0xBB,0xAA};
	uint8_t TxData[] = "Hello World\n";
	NRF_ret_val_EN = NRF24_Init_EN(NRF_HAL_function_local_STR);
	if (NRF_ret_val_EN != NRF_OK_EN)
	{
		while(1);
	}
	NRF_ret_val_EN = NRF24_TxMode_EN(TxAddress, 10);
	if (NRF_ret_val_EN != NRF_OK_EN)
	{
		while(1);
	}

	NRF_ret_val_EN = NRF24_ReadAll_EN(data);
	if (NRF_ret_val_EN != NRF_OK_EN)
	{
		while(1);
	}
	while(1)
	{
		if (NRF24_Transmit_EN(TxData,12) != NRF_OK_EN)
		{
			while(1);
		}

		HAL_Delay(1000);
	}
}*/






