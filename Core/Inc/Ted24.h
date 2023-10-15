/*
 * Ted24.h
 *
 *  Created on: Oct 14, 2023
 *      Author: teddy
 */

#ifndef INC_TED24_H_
#define INC_TED24_H_

#include "NRF24L01.h"
#include <stdint.h>

#define cSIZE_PAYLOAD_U8 ((uint8_t)cSIZE_BUFFER_TX_MAX_U8-12)

typedef enum
{
	TED_NOT_INIT_EN,
	TED_SEND_PACKET_NOT_OK_EN,
	TED_TX_MODE_UNAVAILABLE_EN,
	TED_RX_MODE_UNAVAILABLE_EN,
	TED_OK_EN
}TED_ret_val_en;

typedef union
{
	struct packet_str {
		uint8_t version_Ted24_U4:4;
		uint8_t ID_reseau_U4:4;
	    uint8_t address_emetteur[8];
	    uint8_t address_Destinataire;
	    uint8_t nb_nodes_traverses_U3:3;
	    uint8_t function_U5:5;
	    uint8_t payload_U8A[cSIZE_PAYLOAD_U8];
	    uint8_t crc8_Id_paquet_U8;
	} packet_STR;
    uint8_t packet_U8A[cSIZE_BUFFER_TX_MAX_U8];
}TED_packet_un;


// /!\ jusqu'a 32 functions
typedef enum
{
	PING=0b00001,
	ACK=0b00010
}TED_function_en;


bool TED_IsDataAvailable_B(void);
TED_ret_val_en TED_ping_EN(uint8_t address_dst_U8);
TED_ret_val_en TED_init(uint8_t my_address_U8,NRF_HAL_function_str NRF_HAL_function_STR,bool flag_activating_low_power_mode_B);
TED_ret_val_en TED_receive_EN(TED_packet_un* TED_packet_UN);
void print_rx_packet_with_string_payload(TED_packet_un TED_packet_UN);
TED_ret_val_en TED_ack_EN(uint8_t address_dst_U8, TED_function_en function_to_ack_EN);





#endif /* INC_TED24_H_ */
