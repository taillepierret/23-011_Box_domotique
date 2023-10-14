/*
 * Ted24.c
 *
 *  Created on: Oct 14, 2023
 *      Author: teddy
 */

//TODO etudier pour le bootloader
//TODO faire la partie mesh du réseau
//TODO faire une machine a etat pour l'envoie de commandes
//TODO ajouter la verification que l'init est faite un peu partout sauf dans les fcts static

#include "Ted24.h"
#include "tools.h"

uint8_t PipeAddress[] = {0xEE,0xDD,0xCC,0xBB,0xAA};

typedef struct
{
	uint8_t my_address_U8;
	bool flag_activating_low_power_mode_B;
	bool flag_node_is_init_B;;
}TED_config_node_str;

TED_config_node_str local_TED_config_node_TED = {0};

TED_ret_val_en TED_init(uint8_t my_address_U8,NRF_HAL_function_str NRF_HAL_function_STR,bool flag_activating_low_power_mode_B)
{
	NRF_ret_val_en NRF_ret_val_EN;
	NRF_ret_val_EN = NRF24_RxMode_EN(PipeAddress, 10);
	if(NRF_ret_val_EN != NRF_OK_EN)
	{
		return TED_NOT_INIT_EN;
	}
	NRF_ret_val_EN = NRF24_Init_EN(NRF_HAL_function_STR);
	if(NRF_ret_val_EN != NRF_OK_EN)
	{
		return TED_NOT_INIT_EN;
	}
	else
	{
		local_TED_config_node_TED.my_address_U8 = my_address_U8;
		local_TED_config_node_TED.flag_activating_low_power_mode_B = flag_activating_low_power_mode_B;
		local_TED_config_node_TED.flag_node_is_init_B = true;
		return TED_OK_EN;
	}
}

inline bool TED_IsDataAvailable_B(void)
{
	return NRF24_isDataAvailable_EN(1) == NRF_DATA_AVAILABLE_EN;
}

static inline TED_ret_val_en TED_send_packet_EN(TED_packet_un TED_packet_UN)
{
	if (NRF24_Transmit_EN(TED_packet_UN.packet_U8A,cSIZE_BUFFER_TX_MAX_U8) != NRF_OK_EN)
	{
		return TED_SEND_PACKET_NOT_OK_EN;
	}
	else
	{
	return TED_OK_EN;
	}
}


TED_ret_val_en TED_ping_EN(uint8_t address_dst_U8)
{
	TED_ret_val_en TED_ret_val_EN;
	NRF_ret_val_en NRF_ret_val_EN;
	TED_packet_un TED_packet_to_send_UN = {
			.packet_STR.address_Destinataire = address_dst_U8,
			.packet_STR.address_emetteur[0] = local_TED_config_node_TED.my_address_U8,
			.packet_STR.function_U5 = PING,
			.packet_STR.nb_nodes_traversés_U3 = 0b000,
			.packet_STR.crc8_Id_paquet_U8 = 0,
			.packet_STR.ID_reseau_U4 = 0b1001,
			.packet_STR.payload_U8A = "Aurelie est si belle"
	};

	TED_packet_to_send_UN.packet_STR.crc8_Id_paquet_U8 = calculate_crc8_U8(TED_packet_to_send_UN.packet_U8A, cSIZE_BUFFER_TX_MAX_U8);
	NRF_ret_val_EN = NRF24_TxMode_EN(PipeAddress, 10);
	if (NRF_ret_val_EN != NRF_OK_EN)
	{
		return TED_TX_MODE_UNAVAILABLE_EN;
	}
	TED_ret_val_EN = TED_send_packet_EN(TED_packet_to_send_UN);
	if (TED_ret_val_EN != TED_OK_EN)
	{
		return TED_ret_val_EN;
	}
	NRF_ret_val_EN = NRF24_RxMode_EN(PipeAddress, 10);
	if (NRF_ret_val_EN != NRF_OK_EN)
	{
		return TED_RX_MODE_UNAVAILABLE_EN;
	}
	else
	{
		return TED_OK_EN;
	}
}

static inline void print_rx_packet_with_string_payload(TED_packet_un TED_packet_UN)
{
	char string[cSIZE_BUFFER_TX_MAX_U8-12] = "";
	for (uint8_t index_U8=0 ; index_U8<cSIZE_BUFFER_TX_MAX_U8-12 ; index_U8++)
	{
		string[index_U8] = TED_packet_UN.packet_STR.payload_U8A[index_U8];
	}
	print_string(string);
}

inline TED_ret_val_en TED_receive_EN(TED_packet_un TED_packet_UN)
{
	NRF_ret_val_en NRF_ret_val_EN;
	NRF_ret_val_EN = NRF24_Receive_EN(TED_packet_UN.packet_U8A);
	if (NRF_ret_val_EN != NRF_OK_EN)
	{
		return TED_RX_MODE_UNAVAILABLE_EN;
	}
	else
	{
		return TED_OK_EN;
	}
}


