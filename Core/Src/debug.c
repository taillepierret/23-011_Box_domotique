/*
 * debug.c
 *
 *  Created on: Oct 28, 2023
 *      Author: teddy
 */


#include "debug.h"
#include "hal.h"
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
