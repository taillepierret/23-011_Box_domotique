/*
 * Application.c
 *
 *  Created on: Sep 30, 2023
 *      Author: teddy
 */

#include "string.h"
#include "hal.h"
#include "tools.h"
#include "Ted24.h"
#include "NRF24L01.h"
#include "configuration.h"
#include "debug.h"
#include "Application.h"
#include <stdlib.h>

static void APP_addDebugCommand(void);
void APP_sendPingFromDBG(char command_CA[cSIZE_MAX_BUFFER_DEBUG_U16]);

#define SERVEUR
#ifndef SERVEUR
#define RX_MODE
#endif

const static NRF_HAL_function_str NRF_HAL_function_local_STR =
{
	.readSpiValue_EN_PF = HAL_readSpiValue_EN,
	.setCe_PF = HAL_setCE,
	.setIrq_PF = HAL_setIRQ,
	.writeSpiValue_EN_PF = HAL_writeSpiValue_EN
};

#ifdef SERVEUR
uint8_t my_addr = 69;
uint8_t addr_dst = 14;
#else
uint8_t my_addr = 14;
uint8_t addr_dst = 69;
#endif
TED_packet_un TED_packet_UN;

void runApp(void)
{
	DBG_setLogLevel(VERBOSE_EN);
	TED_init(my_addr, cNETWORK_ID_U8, NRF_HAL_function_local_STR,true);
	APP_addDebugCommand();
	HAL_enableRxDmaUart2();
#ifndef SERVEUR
	TED_ping_EN(addr_dst);
	TED_ping_EN(addr_dst);
	TED_ping_EN(addr_dst);
	TED_ping_EN(addr_dst);
	TED_ping_EN(addr_dst);
	TED_ping_EN(addr_dst);
#endif
	while(1)
	{
		Ted_Process();
		DBG_process();
	}
}

static void APP_addDebugCommand(void)
{
	DBG_addDebugCommand("<GET COUNTER ERROR>",TED_printCounterErrorValue,NULL);
	DBG_addDebugCommand("<SEND PING TO:",NULL,APP_sendPingFromDBG);
}

void APP_sendPingFromDBG(char command_CA[cSIZE_MAX_BUFFER_DEBUG_U16])
{
	uint16_t begin_index_U16 = 0;
	uint16_t ending_index_U16 = 0;
	uint8_t nb_char_address_value__U8 = 0;
	bool flag_is_there_a_begin = false;
	bool flag_is_there_an_end = false;
	char address_value_CA[3] = {0};
	uint16_t address_value_U16 = 0;

	for(uint16_t counter_U16=0 ; counter_U16<cSIZE_MAX_BUFFER_DEBUG_U16 ; counter_U16++)
	{
		if(command_CA[counter_U16] == '>')
		{
			flag_is_there_an_end = true;
			ending_index_U16 = counter_U16;
		}
		else if(command_CA[counter_U16] == ':')
		{
			flag_is_there_a_begin = true;
			begin_index_U16 = counter_U16;
		}
		else if(flag_is_there_a_begin == true && flag_is_there_an_end == false)
		{
			if(command_CA[counter_U16] != '0' && command_CA[counter_U16] != '1' && command_CA[counter_U16] != '2' && command_CA[counter_U16] != '3' && command_CA[counter_U16] != '4' &&
			   command_CA[counter_U16] != '5' && command_CA[counter_U16] != '6' && command_CA[counter_U16] != '7' && command_CA[counter_U16] != '8' && command_CA[counter_U16] != '9')
			{
				DBG_printString("<ADDRESS VALUE IS NOT A NUMBER>\r\n", ERROR_EN);
				return;
			}
		}
		if (flag_is_there_an_end && flag_is_there_a_begin)
		{
			break;
		}
	}
	nb_char_address_value__U8 = (uint8_t)(ending_index_U16-begin_index_U16-1);
	if(nb_char_address_value__U8>3)
	{
		DBG_printString("<ADDRESS VALUE IS TOO LONG>\r\n", ERROR_EN);
		return;
	}
	for(uint8_t counter_U8=0 ; counter_U8<nb_char_address_value__U8 ; counter_U8++)
	{
		address_value_CA[counter_U8] = command_CA[begin_index_U16+1+counter_U8];
	}
	address_value_CA[nb_char_address_value__U8] = '\0';
	address_value_U16 = atoi(address_value_CA);
	if (address_value_U16>0xFF) //si l'adresse n'est pas codée sur 8bits
	{
		DBG_printString("<ADDRESS VALUE IS TOO LONG>\r\n", ERROR_EN);
		return;
	}
	TED_ping_EN((uint8_t)address_value_U16);
	DBG_printString("<PING SEND TO:", ERROR_EN);
	DBG_printUint32_t(address_value_U16, ERROR_EN);
	DBG_printString(">\r\n", ERROR_EN);
}


