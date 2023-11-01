/*
 * debug.c
 *
 *  Created on: Oct 28, 2023
 *      Author: teddy
 */

//TODO ajouter un commande pour activer et desactiver les command de debug
//TODO separer les commandes de debug et de log

#include "debug.h"
#include <stddef.h>
#include <string.h>

#define cSIZE_OF_COMMAND_DEBUG_LIST_U8 ((uint8_t) 4)


typedef struct{
	char command_CA[cSIZE_MAX_BUFFER_DEBUG_U16];
	void (*functionToExecuteWithoutParam_FP)(void);
	void (*functionToExecuteWithParam_FP)(char command_CA[cSIZE_MAX_BUFFER_DEBUG_U16]);
}DBG_command_str;

static DBG_command_str DBG_list_command_STRA[cSIZE_OF_COMMAND_DEBUG_LIST_U8] = {0};
static uint8_t counter_of_command_debug_U8 = 0;

//TODO ajouter les fonction de debug UART
DBG_log_level_en local_DBG_log_level_EN = ERROR_EN;

inline void DBG_setLogLevel(DBG_log_level_en DBG_log_level_EN)
{
	local_DBG_log_level_EN = DBG_log_level_EN;
}

inline void DBG_printString(char* log_CA,DBG_log_level_en DBG_log_level_EN)
{
	if (local_DBG_log_level_EN<=DBG_log_level_EN)
	{
		print_string(log_CA);
	}
}

inline void DBG_printUint32_t(uint32_t number_U32,DBG_log_level_en DBG_log_level_EN)
{
	if (local_DBG_log_level_EN<=DBG_log_level_EN)
	{
		print_uint32(number_U32);
	}
}

bool DBG_checkIfPacketAsBeginAndEnd_B(char DBG_raw_buffer_rx_CA[cSIZE_MAX_BUFFER_DEBUG_U16])
{
	bool flag_is_there_a_begin = false;
	bool flag_is_there_an_end = false;
	for (uint8_t index_U8=0 ; index_U8<cSIZE_MAX_BUFFER_DEBUG_U16 ; index_U8++)
	{
		if(DBG_raw_buffer_rx_CA[index_U8] == '>')
		{
			flag_is_there_an_end = true;
		}
		else if(DBG_raw_buffer_rx_CA[index_U8] == '<')
		{
			flag_is_there_a_begin = true;
		}
		if (flag_is_there_an_end && flag_is_there_a_begin)
		{
			return true;
		}
	}
	return false;
}

/**
 * @fn DBG_ret_val_en DBG_ManageActualRawRxPacket(void)
 * @brief find the beginning of the packet and put it in [0] and so on for the rest. it is doing this for the packet whiwh will be treat,
 * so the counter_buffer_rx_treated_U8 one (think to counter_buffer_rx_treated_U8++ before calling the function
 *
 * @return
 */
static DBG_ret_val_en DBG_ManageRxPacket(uint8_t rx_buffer_U8A[cSIZE_BUFFER_UART_2_RX_U16])
{
	uint16_t index_the_packet_s_end = cSIZE_MAX_BUFFER_DEBUG_U16+1;
	uint16_t index_the_packet_s_begin = cSIZE_MAX_BUFFER_DEBUG_U16+1;
	uint16_t size_rx_packet_U16 = 0;
	char DBG_managed_buffer_rx_CA[cSIZE_MAX_BUFFER_DEBUG_U16];
	bool flag_is_there_a_begin = false;
	bool flag_is_there_an_end = false;

	for(uint16_t counter_U16=0 ; counter_U16<cSIZE_MAX_BUFFER_DEBUG_U16 ; counter_U16++)
	{
		if(rx_buffer_U8A[counter_U16] == '>')
		{
			flag_is_there_an_end = true;
			index_the_packet_s_end = counter_U16;
		}
		else if(rx_buffer_U8A[counter_U16] == '<')
		{
			flag_is_there_a_begin = true;
			index_the_packet_s_begin = counter_U16;
		}
		if (flag_is_there_an_end && flag_is_there_a_begin)
		{
			break;
		}
	}
	if(index_the_packet_s_begin<index_the_packet_s_end)
	{
		size_rx_packet_U16 = index_the_packet_s_end-index_the_packet_s_begin+1;
	}
	else
	{
		size_rx_packet_U16 = cSIZE_MAX_BUFFER_DEBUG_U16-index_the_packet_s_begin+index_the_packet_s_end+1;
	}
	//remise du packet recu dans le bon sens avec '<' en [0]
	for(uint16_t counter_U16=0 ; counter_U16<size_rx_packet_U16 ; counter_U16++)
	{
		if(index_the_packet_s_begin+counter_U16 < cSIZE_MAX_BUFFER_DEBUG_U16) //on verifie si index_the_packet_s_end-counter_U16 n'est pas négatif /!\ on est en unsigned
		{
			DBG_managed_buffer_rx_CA[counter_U16] = rx_buffer_U8A[index_the_packet_s_begin+counter_U16];
		}
		else
		{
			DBG_managed_buffer_rx_CA[counter_U16] = rx_buffer_U8A[index_the_packet_s_begin+counter_U16-cSIZE_MAX_BUFFER_DEBUG_U16];
		}
	}

	for (uint8_t index_U8=0 ; index_U8<size_rx_packet_U16 ; index_U8++)
	{
		rx_buffer_U8A[index_U8] = DBG_managed_buffer_rx_CA[index_U8];
	}
	rx_buffer_U8A[size_rx_packet_U16] = '\0';
	return DBG_PACKET_RX_GOOD_EN;
}


//TODO verifier qu'une commande s'est bien executee ou pas en renvoyant un bool sur la commande
DBG_ret_val_en DBG_treatCommand(char* command_CA)
{
	for (uint8_t counter_U8=0 ; counter_U8<cSIZE_OF_COMMAND_DEBUG_LIST_U8 ; counter_U8++)
	{
		if(memcmp(command_CA,DBG_list_command_STRA[counter_U8].command_CA,strlen(DBG_list_command_STRA[counter_U8].command_CA)) == 0)
		{
			if(DBG_list_command_STRA[counter_U8].functionToExecuteWithParam_FP != NULL)
			{
				DBG_list_command_STRA[counter_U8].functionToExecuteWithParam_FP(command_CA);
			}
			else if(DBG_list_command_STRA[counter_U8].functionToExecuteWithoutParam_FP != NULL)
			{
				DBG_list_command_STRA[counter_U8].functionToExecuteWithoutParam_FP();
			}
			else
			{
				DBG_printString("<COMMAND DOES NOT HAVE FUNCTION>\r\n", ERROR_EN);
				return DBG_COMMAND_WITHOUT_FUNCTION_EN;
			}
			DBG_printString("<COMMAND IS TREAT SUCCESFULLY>\r\n\r\n", ERROR_EN);
			return DBG_OK_EN;
		}
	}
	DBG_printString("<COMMAND NOT FOUND>\r\n", ERROR_EN);
	return DBG_COMMAND_NOT_FOUND_EN;
}


void DBG_addDebugCommand(char* command_CA,
						void (*functionToExecuteWithoutParam_FP)(void),
						void (*functionToExecuteWithParam_FP)(char* command_CA))
{
	for (uint16_t counter_U16=0 ; counter_U16<cSIZE_MAX_BUFFER_DEBUG_U16 ; counter_U16++)
	{
		DBG_list_command_STRA[counter_of_command_debug_U8].command_CA[counter_U16] = command_CA[counter_U16];
		if(command_CA[counter_U16] == '\0')
		{
			break;
		}
	}
	DBG_list_command_STRA[counter_of_command_debug_U8].functionToExecuteWithParam_FP = functionToExecuteWithParam_FP;
	DBG_list_command_STRA[counter_of_command_debug_U8].functionToExecuteWithoutParam_FP = functionToExecuteWithoutParam_FP;
	counter_of_command_debug_U8++;
	if(counter_of_command_debug_U8>=cSIZE_OF_COMMAND_DEBUG_LIST_U8)
	{
		while(1);
	}
}

void printBufferUart2(void)
{
	uint8_t rx_buffer_U8A[cSIZE_BUFFER_UART_2_RX_U16];
	HAL_getUart2Buffer(rx_buffer_U8A);
	DBG_printString((char*)rx_buffer_U8A, ERROR_EN);
	DBG_printString("\r\n", ERROR_EN);
}

void DBG_process(void)
{
	uint8_t rx_buffer_U8A[cSIZE_BUFFER_UART_2_RX_U16];
	HAL_getUart2Buffer(rx_buffer_U8A);
	if(DBG_checkIfPacketAsBeginAndEnd_B((char*)rx_buffer_U8A) == true)
	{
		HAL_flushUart2Buffer();
		DBG_ret_val_en DBG_ret_val_EN;
		DBG_ret_val_EN = DBG_ManageRxPacket(rx_buffer_U8A);
		if (DBG_ret_val_EN == DBG_PACKET_RX_GOOD_EN)
		{
			DBG_treatCommand((char*)rx_buffer_U8A);
		}
	}
}




