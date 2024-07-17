/*
 * tools.h
 *
 *  Created on: Sep 18, 2023
 *      Author: teddy
 */

#ifndef INC_TOOLS_H_
#define INC_TOOLS_H_

#include <stdint.h>

#define cINT_TO_STRING_OFFSET_U8 ((uint8_t) 0x30)

uint8_t calculate_crc8_U8(const uint8_t *data_U8A, uint8_t length_U8);






#endif /* INC_TOOLS_H_ */
