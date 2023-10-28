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

#define cPROTOCOL_VERSION_U8 (uint8_t) 0

#define cSIZE_PAYLOAD_U8 ((uint8_t)cSIZE_BUFFER_TX_MAX_U8-12)

typedef enum
{
	TED_NOT_INIT_EN,
	TED_SEND_PACKET_NOT_OK_EN,
	TED_TX_MODE_UNAVAILABLE_EN,
	TED_RX_MODE_UNAVAILABLE_EN,
	TED_NO_TASK_RUNNING_EN,
	TED_WRONG_TASK_EN,
	TED_SENDING_STACK_OVERFLOW_EN,
	TED_PAQCKET_SENDED_WITHOUT_ACK_RECEIVED_EN,
	TED_PAQCKET_SENDED_AND_ACK_RECEIVED_EN,
	TED_WRONG_NETWORK_ID_EN,
	TED_WRONG_DST_ADR_EN,
	TED_WRONG_CRC_EN,
	TED_WRONG_PROTOCOL_VERSION_EN,
	TED_COMMAND_NOT_FOUND_EN,
	TED_ACK_IS_RECEIVED,
	TED_TIMEOUT_ACK_IS_NOT_RECEIVED,
	TED_WAITING_FOR_ACK,
	TED_ACK_RECEIVED_NOT_CORRESPONDING_TO_ACTUAL_TX_PACKET,
	TED_NUMBER_OF_RETRY_TOO_BIG_EN,
	TED_RETRY_EN,
	TED_OK_EN
}TED_ret_val_en;

//TODO voir si le [0] c'est le crc8, si c'est le cas, il faut mettre au plius proche de [0] les valeurs les moins susceptibles de changer pour les futurs version du protocole
typedef union
{
	struct packet_str {
		uint8_t version_Ted24_U4:4;
		uint8_t ID_reseau_U4:4;
	    uint8_t address_Destinataire_U8;
	    uint8_t nb_nodes_traverses_U3:3;
	    uint8_t function_U5:5;
	    uint8_t payload_U8A[cSIZE_PAYLOAD_U8];
	    uint8_t address_emetteur_U8A[8]; //TODO /!\ le crc se calcul sans le crc ni les adresses emettrices
	    uint8_t crc8_Id_paquet_U8;
	} packet_STR;
    uint8_t packet_U8A[cSIZE_BUFFER_TX_MAX_U8];
}TED_packet_un;


// /!\ jusqu'a 32 functions max
typedef enum
{
	PING=0b00001,
	ACK=0b00010
}TED_function_en;

typedef enum
{
	SENDING_PACKET_TASK,
	WAITING_FOR_ACK_TASK,
	RETRY_SENDING_TASK,
	NO_TASK
}TED_task_en;


bool TED_IsDataAvailable_B(void);
TED_ret_val_en TED_ping_EN(uint8_t address_dst_U8);
TED_ret_val_en TED_init(uint8_t my_address_U8,uint8_t ID_network_U8,NRF_HAL_function_str NRF_HAL_function_STR,bool flag_activating_low_power_mode_B);
TED_ret_val_en TED_receive_EN(TED_packet_un* TED_packet_UN);
void print_rx_packet_with_string_payload(TED_packet_un TED_packet_UN);
TED_ret_val_en TED_ack_EN(uint8_t address_dst_U8, TED_function_en function_to_ack_EN, uint8_t Id_packet_to_ack);
void Ted_Process(void);





#endif /* INC_TED24_H_ */
