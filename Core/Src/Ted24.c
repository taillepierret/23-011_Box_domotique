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
//TODO historiser les problemes rencontrés
//TODO faire la fonction millis()
//TODO faire un compteur de paquets supprimés
//TODO n'ajouter au la liste de paquets RX que les paquets qui nous sont destiné
//TODO enregistrer tous les logs dans l'EEPROM et afficher une led rouge quadn l'EEPROM est full, faire une commande pour envoyer les logs en debug et en faire une autre pour tout flusher

#include "Ted24.h"
#include "tools.h"


#define cSIZE_STACK_PACKET_TO_SEND_U8 ((uint8_t) 20)
#define cSIZE_STACK_PACKET_REVEIVED_U8 ((uint8_t) 20)
#define cTIMEOUT_WAITING_FOR_ACK_ms_U32 ((uint32_t) 500)

typedef struct
{
	uint8_t my_address_U8;
	bool flag_activating_low_power_mode_B;
	bool flag_node_is_init_B;
	uint8_t network_ID_U4:4;
	uint8_t protocol_version_U4:4;
}TED_config_node_str;

static const uint8_t PipeAddress[] = {0xEE,0xDD,0xCC,0xBB,0xAA};
static TED_config_node_str local_TED_config_node_TED = {.my_address_U8 = 0,
														.flag_activating_low_power_mode_B = false,
														.flag_node_is_init_B = false,
														.network_ID_U4 = 0,
														.protocol_version_U4 = cPROTOCOL_VERSION_U8};
static TED_packet_un liste_de_paquets_a_envoyer_ENA[cSIZE_STACK_PACKET_TO_SEND_U8] = {0};
static uint8_t counter_packet_sended_U8 = 0;
static uint8_t counter_packet_added_U8 = 0;
typedef struct{
	TED_packet_un TED_packet_UN;
	uint32_t received_time_ms_U32;
}TED_Rx_packet_STR;
static TED_Rx_packet_STR liste_de_paquets_recus_ENA[cSIZE_STACK_PACKET_REVEIVED_U8] = {0};


static uint8_t counter_packet_received_U8 = 0;
static uint8_t counter_packet_treated_U8 = 0;
static TED_task_en tache_en_cours_EN = NO_TASK;

TED_ret_val_en TED_init(uint8_t my_address_U8,uint8_t ID_network_U8,NRF_HAL_function_str NRF_HAL_function_STR,bool flag_activating_low_power_mode_B)
{
	//TODO verifier que ID_network_U8 ne depasse pas 4bits
	NRF_ret_val_en NRF_ret_val_EN;
	NRF_ret_val_EN = NRF24_Init_EN(NRF_HAL_function_STR);
	if(NRF_ret_val_EN != NRF_OK_EN)
	{
		return TED_NOT_INIT_EN;
	}
	NRF_ret_val_EN = NRF24_RxMode_EN((uint8_t *)PipeAddress, 10);
	if(NRF_ret_val_EN != NRF_OK_EN)
	{
		return TED_NOT_INIT_EN;
	}
	else
	{
		local_TED_config_node_TED.my_address_U8 = my_address_U8;
		local_TED_config_node_TED.flag_activating_low_power_mode_B = flag_activating_low_power_mode_B;
		local_TED_config_node_TED.flag_node_is_init_B = true;
		local_TED_config_node_TED.network_ID_U4 = ID_network_U8;
		return TED_OK_EN;
	}
}

inline bool TED_IsDataAvailable_B(void)
{
	return NRF24_isDataAvailable_EN(1) == NRF_DATA_AVAILABLE_EN;
}

static inline TED_ret_val_en TED_packet_to_send_EN(uint8_t addr_dst_U8, TED_function_en function_EN, uint8_t payload_U8A[cSIZE_PAYLOAD_U8])
{
	TED_packet_un TED_packet_to_send_UN = {
			.packet_STR.address_Destinataire_U8 = addr_dst_U8,
			.packet_STR.address_emetteur_U8A[0] = local_TED_config_node_TED.my_address_U8,
			.packet_STR.function_U5 = function_EN,
			.packet_STR.nb_nodes_traverses_U3 = 0b000,
			.packet_STR.crc8_Id_paquet_U8 = 0,
			.packet_STR.ID_reseau_U4 = 0b1001,
	};
	for (uint8_t index_U8=0 ; index_U8<cSIZE_PAYLOAD_U8 ; index_U8++)
	{
		TED_packet_to_send_UN.packet_STR.payload_U8A[index_U8] = payload_U8A[index_U8];
	}
	TED_packet_to_send_UN.packet_STR.crc8_Id_paquet_U8 = calculate_crc8_U8(TED_packet_to_send_UN.packet_U8A, cSIZE_BUFFER_TX_MAX_U8-1);
	counter_packet_added_U8++;

	//gestion de la stack tournante
	if (counter_packet_added_U8>=cSIZE_STACK_PACKET_TO_SEND_U8)
	{
		counter_packet_added_U8 = 0;
	}

	//securité pour eviter d'écrire sur des paquets pas encore envoyes
	if (counter_packet_added_U8 == counter_packet_sended_U8)
	{
		if (counter_packet_added_U8 == 0)
		{
			counter_packet_added_U8 = cSIZE_STACK_PACKET_TO_SEND_U8-1;
		}
		else
		{
			counter_packet_added_U8--;
		}
		return TED_SENDING_STACK_OVERFLOW_EN;
	}

	//copie du paquet à envoyer dans la stack de paquets à envoyer
	for (uint8_t index_U8=0 ; index_U8<cSIZE_BUFFER_TX_MAX_U8 ; index_U8++)
	{
		liste_de_paquets_a_envoyer_ENA[counter_packet_added_U8].packet_U8A[index_U8] = TED_packet_to_send_UN.packet_U8A[index_U8];
	}
	return NRF_OK_EN;
}


TED_ret_val_en TED_ping_EN(uint8_t address_dst_U8)
{
	TED_ret_val_en TED_ret_val_EN;
	NRF_ret_val_en NRF_ret_val_EN;
	uint8_t payload[cSIZE_PAYLOAD_U8] = "Aurelie est si belle";

	NRF_ret_val_EN = NRF24_TxMode_EN((uint8_t *)PipeAddress, 10);
	if (NRF_ret_val_EN != NRF_OK_EN)
	{
		return TED_TX_MODE_UNAVAILABLE_EN;
	}
	TED_ret_val_EN = TED_packet_to_send_EN(address_dst_U8,PING,payload);
	if (TED_ret_val_EN != TED_OK_EN)
	{
		return TED_ret_val_EN;
	}
	NRF_ret_val_EN = NRF24_RxMode_EN((uint8_t *)PipeAddress, 10);
	if (NRF_ret_val_EN != NRF_OK_EN)
	{
		return TED_RX_MODE_UNAVAILABLE_EN;
	}
	else
	{
		return TED_OK_EN;
	}
}

inline void print_rx_packet_with_string_payload(TED_packet_un TED_packet_UN)
{
	char string[cSIZE_BUFFER_TX_MAX_U8-12+1] = "";
	for (uint8_t index_U8=0 ; index_U8<cSIZE_BUFFER_TX_MAX_U8-12 ; index_U8++)
	{
		string[index_U8] = TED_packet_UN.packet_STR.payload_U8A[index_U8];
	}
	print_string("Version du reseau TED24: ");
	print_uint32(TED_packet_UN.packet_STR.version_Ted24_U4);
	print_string("\r\n");

	print_string("ID du reseau TED24: ");
	print_uint32(TED_packet_UN.packet_STR.ID_reseau_U4);
	print_string("\r\n");

	print_string("Adresse emetteur: ");
	print_uint32(TED_packet_UN.packet_STR.address_emetteur_U8A[0]);
	print_string("\r\n");

	print_string("Adresse destinataire: ");
	print_uint32(TED_packet_UN.packet_STR.address_Destinataire_U8);
	print_string("\r\n");

	print_string("Fonction: ");
	print_uint32(TED_packet_UN.packet_STR.function_U5);
	print_string("\r\n");

	print_string("Payload: ");
	print_string(string);
	print_string("\r\n");

	print_string("CRC recu: ");
	print_uint32(TED_packet_UN.packet_STR.crc8_Id_paquet_U8);
	print_string("\r\n");

	TED_packet_UN.packet_STR.crc8_Id_paquet_U8=0;

	print_string("CRC calcule: ");
	print_uint32(calculate_crc8_U8(TED_packet_UN.packet_U8A,cSIZE_BUFFER_TX_MAX_U8-1));
	print_string("\r\n");
	print_string("\r\n");
}

inline TED_ret_val_en TED_receive_EN(TED_packet_un* TED_packet_UN)
{
	NRF_ret_val_en NRF_ret_val_EN;
	NRF_ret_val_EN = NRF24_Receive_EN(TED_packet_UN->packet_U8A);
	if (NRF_ret_val_EN != NRF_OK_EN)
	{
		return TED_RX_MODE_UNAVAILABLE_EN;
	}
	else
	{
		return TED_OK_EN;
	}
}



inline TED_ret_val_en TED_ack_EN(uint8_t address_dst_U8, TED_function_en function_to_ack_EN)
{
	//TODO renvoyer dans le payload toutes les adresses par lesquelles le paquet a ack est passe
	TED_ret_val_en TED_ret_val_EN;
	NRF_ret_val_en NRF_ret_val_EN;
	uint8_t payload[cSIZE_PAYLOAD_U8] = {function_to_ack_EN};

	NRF_ret_val_EN = NRF24_TxMode_EN((uint8_t *)PipeAddress, 10);
	if (NRF_ret_val_EN != NRF_OK_EN)
	{
		return TED_TX_MODE_UNAVAILABLE_EN;
	}
	TED_ret_val_EN = TED_packet_to_send_EN(address_dst_U8,ACK,payload);
	if (TED_ret_val_EN != TED_OK_EN)
	{
		return TED_ret_val_EN;
	}
	NRF_ret_val_EN = NRF24_RxMode_EN((uint8_t *)PipeAddress, 10);
	if (NRF_ret_val_EN != NRF_OK_EN)
	{
		return TED_RX_MODE_UNAVAILABLE_EN;
	}
	else
	{
		return TED_OK_EN;
	}
}

TED_ret_val_en TED_processTxPacket (void)
{
	NRF_ret_val_en NRF_ret_val_EN;
	if(tache_en_cours_EN == NO_TASK && counter_packet_sended_U8 != counter_packet_added_U8)
	{
		tache_en_cours_EN = SENDING_PACKET_TASK;
	}
	switch (tache_en_cours_EN)
	{
		case SENDING_PACKET_TASK:
			counter_packet_sended_U8++;
			if(counter_packet_sended_U8>=cSIZE_STACK_PACKET_TO_SEND_U8)
			{
				counter_packet_sended_U8 = 0;
			}
			NRF_ret_val_EN = NRF24_Transmit_EN(liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].packet_U8A,cSIZE_BUFFER_TX_MAX_U8); //envoie paquet TED24
			if (NRF_ret_val_EN != NRF_OK_EN)
			{
				return TED_SEND_PACKET_NOT_OK_EN;
			}
			else
			{
				return TED_OK_EN;
			}
		case WAITING_FOR_ACK_TASK:
			//TODO mettre un timeout si je ne recois pas de paquet
		case NO_TASK:
			return TED_NO_TASK_RUNNING_EN;
		default:
			return TED_WRONG_TASK_EN;

	}
}

TED_ret_val_en IsPacketForMe_B (TED_packet_un TED_packet_UN)
{
	if (TED_packet_UN.packet_STR.ID_reseau_U4 == local_TED_config_node_TED.network_ID_U4)
	{
		return TED_WRONG_NETWORK_ID_EN;
	}
	else if (TED_packet_UN.packet_STR.address_Destinataire_U8 == local_TED_config_node_TED.my_address_U8)
	{
		return TED_WRONG_DST_ADR_EN;
	}
	else if (TED_packet_UN.packet_STR.crc8_Id_paquet_U8 == calculate_crc8_U8(TED_packet_UN.packet_U8A,cSIZE_BUFFER_TX_MAX_U8-1))
	{
		return TED_WRONG_CRC_EN;
	}
	else if (TED_packet_UN.packet_STR.version_Ted24_U4 == local_TED_config_node_TED.protocol_version_U4)
	{
		return TED_WRONG_PROTOCOL_VERSION_EN;
	}
	//TODO else if le paquet est deja recu
	else
	{
		return TED_OK_EN;
	}
}

void TED_processRxPacket (void)
{
	if (TED_IsDataAvailable_B())
	{
		TED_packet_un TED_Rx_packet_UN;
		TED_receive_EN(&TED_Rx_packet_UN);
		print_rx_packet_with_string_payload(TED_Rx_packet_UN);
		if(IsPacketForMe_B(TED_Rx_packet_UN) == TED_OK_EN)
		{
			counter_packet_received_U8++;
			if (counter_packet_received_U8>=cSIZE_STACK_PACKET_REVEIVED_U8)
			{
				counter_packet_received_U8 = 0;
			}
			//TODO loguer si jamais on ecrit sur un paquet non traité
			for (uint8_t index_U8=0 ; index_U8<cSIZE_BUFFER_TX_MAX_U8 ; index_U8++)
			{
				liste_de_paquets_recus_ENA[counter_packet_received_U8].TED_packet_UN.packet_U8A[index_U8] = TED_Rx_packet_UN.packet_U8A[index_U8];
			}
			liste_de_paquets_recus_ENA[counter_packet_received_U8].received_time_ms_U32 = HAL_millis_U32();
		}
		//else if c'est un paquet a renvoyer
	}
	else if (counter_packet_received_U8 != counter_packet_treated_U8)
	{
		counter_packet_treated_U8++;
		//TODO fonction de traitement des commandes recues
	}
	//else if il y a un paquet a traiter
	else
	{
		return;
	}

}

void Ted_Process(void)
{
	TED_processRxPacket();
	TED_processTxPacket();
}


TED_ret_val_en TED_treatRxPacket(TED_packet_un TED_packet_UN)
{
	switch(TED_packet_UN.packet_STR.function_U5)
	{
		case PING:
			TED_ack_EN(TED_packet_UN.packet_STR.address_emetteur_U8A[0], PING);
			break;

		case ACK:
			print_rx_packet_with_string_payload(TED_packet_UN);
			//TODO gerer l'ack
			break;
		default:
			//TODO afficher en debug que la fonction n'a pas ete reconnue
			return TED_COMMAND_NOT_FOUND_EN;
	}
	return TED_OK_EN;
}



