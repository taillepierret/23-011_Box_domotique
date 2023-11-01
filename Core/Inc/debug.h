/*
 * debug.h
 *
 *  Created on: Oct 28, 2023
 *      Author: teddy
 */

#ifndef INC_DEBUG_H_
#define INC_DEBUG_H_

#include <stdint.h>
#include "hal.h"

#define cSIZE_MAX_BUFFER_DEBUG_U16 cSIZE_BUFFER_UART_2_RX_U16

typedef enum
{
	VERBOSE_EN = 0,
	INFO_EN    = 1,
	ERROR_EN   = 2
}DBG_log_level_en;

typedef enum
{
	DBG_OK_EN,
	DBG_PACKET_RX_GOOD_EN,
	DBG_PACKET_RX_NOT_GOOD_EN,
	DBG_COMMAND_WITHOUT_FUNCTION_EN,
	DBG_COMMAND_NOT_FOUND_EN
}DBG_ret_val_en;

void DBG_printString(char* log_CA,DBG_log_level_en DBG_log_level_EN);
void DBG_printUint32_t(uint32_t number_U32,DBG_log_level_en DBG_log_level_EN);
void DBG_setLogLevel(DBG_log_level_en DBG_log_level_EN);
void DBG_addRxPacketToStack(char DBG_buffer_rx_CA[cSIZE_MAX_BUFFER_DEBUG_U16]);
void DBG_addDebugCommand(char* command_CA,
						void (*functionToExecuteWithoutParam_FP)(void),
						void (*functionToExecuteWithParam_FP)(char* command_CA));
void printBufferUart2(void);
void DBG_process(void);


#endif /* INC_DEBUG_H_ */
