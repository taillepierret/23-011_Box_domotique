/*
 * debug.h
 *
 *  Created on: Oct 28, 2023
 *      Author: teddy
 */

#ifndef INC_DEBUG_H_
#define INC_DEBUG_H_

#include <stdint.h>

typedef enum
{
	VERBOSE_EN = 0,
	INFO_EN    = 1,
	ERROR_EN   = 2
}DBG_log_level_en;

void DBG_printString(char* log_CA,DBG_log_level_en DBG_log_level_EN);
void DBG_printUint32_t(uint32_t number_U32,DBG_log_level_en DBG_log_level_EN);
void DBG_setLogLevel(DBG_log_level_en DBG_log_level_EN);

#endif /* INC_DEBUG_H_ */
