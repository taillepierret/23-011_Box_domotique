/*
 * debug.c
 *
 *  Created on: Oct 28, 2023
 *      Author: teddy
 */


#include "debug.h"
#include "hal.h"

#define cSIZE_MAX_BUFFER_DEBUG_U16 ((uint16_t) 1000)
#define cSIZE_STACK_OF_BUFFER_DEBUG_U8 ((uint8_t) 10)
#define cSIZE_OF_COMMAND_DEBUG_LIST_U8 ((uint8_t) 50)

typedef struct{
	char buffer_rx_CA[cSIZE_MAX_BUFFER_DEBUG_U16];
	uint32_t received_time_ms_U32;
	bool is_good_packet;
}buffer_rx_str;

typedef struct{
	char command_CA[cSIZE_MAX_BUFFER_DEBUG_U16];
	void (*functionToExecuteWithoutParam_FP)(void);
	void (*functionToExecuteWithParam_FP)(char command_CA[cSIZE_MAX_BUFFER_DEBUG_U16]);
}DBG_command_str;

static buffer_rx_str stack_buffer_rx_STRA[cSIZE_STACK_OF_BUFFER_DEBUG_U8] = {0};
static DBG_command_str DBG_list_command_STRA[cSIZE_OF_COMMAND_DEBUG_LIST_U8] = {0};
static uint8_t counter_buffer_rx_treated_U8 = 0;
static uint8_t counter_buffer_received_U8 = 0;
static uint8_t counter_of_command_debug_U8 = 0;

//TODO ajouter les fonction de debug UART
DBG_log_level_en local_DBG_log_level_EN = ERROR_EN;

inline void DBG_setLogLevel(DBG_log_level_en DBG_log_level_EN)
{
	local_DBG_log_level_EN = DBG_log_level_EN;
}

inline void DBG_printString(char* log_CA,DBG_log_level_en DBG_log_level_EN)
{
	if (local_DBG_log_level_EN<DBG_log_level_EN)
	{
		print_string(log_CA);
	}
}

inline void DBG_printUint32_t(uint32_t number_U32,DBG_log_level_en DBG_log_level_EN)
{
	if (local_DBG_log_level_EN<DBG_log_level_EN)
	{
	print_uint32(number_U32);
	}
}

void DBG_addRxPacketToStack(char DBG_buffer_rx_CA[cSIZE_MAX_BUFFER_DEBUG_U16])
{
	counter_buffer_received_U8++;
	if (counter_buffer_received_U8>=cSIZE_MAX_BUFFER_DEBUG_U16)
	{
		counter_buffer_received_U8 = 0;
	}
	//TODO loguer si jamais on ecrit sur un paquet non traité
	for (uint8_t index_U8=0 ; index_U8<cSIZE_MAX_BUFFER_DEBUG_U16 ; index_U8++)
	{
		stack_buffer_rx_STRA[counter_buffer_received_U8].buffer_rx_CA[index_U8] = DBG_buffer_rx_CA[index_U8];
	}
	stack_buffer_rx_STRA[counter_buffer_received_U8].received_time_ms_U32 = HAL_millis_U32();

}

/**
 * @fn DBG_ret_val_en DBG_ManageActualRawRxPacket(void)
 * @brief find the beginning of the packet and put it in [0] and so on for the rest. it is doing this for the packet whiwh will be treat,
 * so the counter_buffer_rx_treated_U8 one (think to counter_buffer_rx_treated_U8++ before calling the function
 *
 * @return
 */
static DBG_ret_val_en DBG_ManageActualRawRxPacket(void)
{
	uint16_t index_the_packet_s_begin = cSIZE_MAX_BUFFER_DEBUG_U16+1;
	uint16_t index_the_packet_s_end = cSIZE_MAX_BUFFER_DEBUG_U16+1;
	bool flag_to_stop_copy = false;
	bool is_there_an_end = false;
	char DBG_managed_buffer_rx_CA[cSIZE_MAX_BUFFER_DEBUG_U16];
	for(uint16_t counter_U16=0 ; counter_U16<cSIZE_MAX_BUFFER_DEBUG_U16 ; counter_U16++)
	{
		if(stack_buffer_rx_STRA[counter_buffer_rx_treated_U8].buffer_rx_CA[counter_U16] == '<')
		{
			index_the_packet_s_begin = counter_U16;
			break;
		}
	}
	if(index_the_packet_s_begin>cSIZE_MAX_BUFFER_DEBUG_U16)
	{
		return DBG_PACKET_RX_NOT_GOOD_EN;
	}
	else
	{
		//remise du packet recu dans le bon sens avec '<' en [0]
		for(uint16_t counter_U16=0 ; counter_U16<cSIZE_MAX_BUFFER_DEBUG_U16 ; counter_U16++)
		{
			if(flag_to_stop_copy == false && index_the_packet_s_begin+counter_U16 < cSIZE_MAX_BUFFER_DEBUG_U16)
			{
				DBG_managed_buffer_rx_CA[counter_U16] = stack_buffer_rx_STRA[counter_buffer_rx_treated_U8].buffer_rx_CA[index_the_packet_s_begin+counter_U16];
				if(DBG_managed_buffer_rx_CA[counter_U16] == '>')
				{
					is_there_an_end = true;
					flag_to_stop_copy = true;
				}
			}
			else if(flag_to_stop_copy == false && index_the_packet_s_begin+counter_U16 >= cSIZE_MAX_BUFFER_DEBUG_U16)
			{
				DBG_managed_buffer_rx_CA[counter_U16] = stack_buffer_rx_STRA[counter_buffer_rx_treated_U8].buffer_rx_CA[index_the_packet_s_begin+counter_U16-cSIZE_MAX_BUFFER_DEBUG_U16];
				if(DBG_managed_buffer_rx_CA[counter_U16] == '>')
				{
					is_there_an_end = true;
					flag_to_stop_copy = true;
					index_the_packet_s_end = counter_U16;
				}
			}
			else
			{
				index_the_packet_s_end++;
				DBG_managed_buffer_rx_CA[counter_U16] = '\0';
				break;
			}
		}
		if(is_there_an_end != true)
		{
			return DBG_PACKET_RX_NOT_GOOD_EN;
		}
		for (uint8_t index_U8=0 ; index_U8<index_the_packet_s_end ; index_U8++)
		{
			stack_buffer_rx_STRA[counter_buffer_rx_treated_U8].buffer_rx_CA[index_U8] = DBG_managed_buffer_rx_CA[index_U8];
		}
		return DBG_PACKET_RX_GOOD_EN;
	}
}


void DBG_addDebugCommand(char command_CA[cSIZE_MAX_BUFFER_DEBUG_U16],
						void (*functionToExecuteWithoutParam_FP)(void),
						void (*functionToExecuteWithParam_FP)(char command_CA[cSIZE_MAX_BUFFER_DEBUG_U16]))
{
	for (uint16_t counter_U16=0 ; counter_U16<cSIZE_MAX_BUFFER_DEBUG_U16 ; counter_U16++)
	{
		DBG_list_command_STRA[counter_of_command_debug_U8].command_CA[counter_U16] = command_CA[counter_U16];
		if(DBG_list_command_STRA[counter_of_command_debug_U8].command_CA[counter_U16] == '\0')
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




