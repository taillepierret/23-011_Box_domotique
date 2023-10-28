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
#include "debug.h"



typedef struct
{
	uint8_t my_address_U8;
	bool flag_activating_low_power_mode_B;
	bool flag_node_is_init_B;
	uint8_t network_ID_U4:4;
	uint8_t protocol_version_U4:4;
}TED_config_node_str;
typedef struct{
	TED_packet_un TED_packet_UN;
	uint32_t received_time_ms_U32;
}TED_Rx_packet_STR;
typedef struct{
	TED_packet_un TED_packet_UN;
	bool flag_is_waiting_for_ack;
	uint32_t begin_waiting_ack_time_ms_U32; //correspond au temps ou le paquet a ete envoye
	uint32_t time_ack_is_received_ms_U32;  //correspond au temps ou le ACK est recu
	bool is_ack_rx;
}TED_Tx_packet_STR;





//----------------------------------------------------- local variables -----------------------------------------------------
#define cSIZE_STACK_PACKET_TO_SEND_U8 ((uint8_t) 20)
#define cSIZE_STACK_PACKET_REVEIVED_U8 ((uint8_t) 20)
#define cWAITING_TIMEOUT_ACK_ms_U32 (uint32_t) 10000

static const uint8_t PipeAddress[] = {0xEE,0xDD,0xCC,0xBB,0xAA};
static TED_config_node_str local_TED_config_node_TED = {.my_address_U8 = 0,
														.flag_activating_low_power_mode_B = false,
														.flag_node_is_init_B = false,
														.network_ID_U4 = 0,
														.protocol_version_U4 = cPROTOCOL_VERSION_U8};
static uint8_t counter_packet_sended_U8 = 0;
static uint8_t counter_packet_added_U8 = 0;


static TED_Rx_packet_STR liste_de_paquets_recus_ENA[cSIZE_STACK_PACKET_REVEIVED_U8] = {0};
static TED_Tx_packet_STR liste_de_paquets_a_envoyer_ENA[cSIZE_STACK_PACKET_TO_SEND_U8] = {0};


static uint8_t counter_packet_received_U8 = 0;
static uint8_t counter_packet_treated_U8 = 0;
static TED_task_en tache_en_cours_EN = NO_TASK;

uint8_t counter_packet_error_U8 = 0;

//----------------------------------------------------- local functions -----------------------------------------------------
static TED_ret_val_en TED_treatRxPacket(TED_packet_un TED_packet_UN);

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
	TED_packet_to_send_UN.packet_STR.crc8_Id_paquet_U8 = calculate_crc8_U8(TED_packet_to_send_UN.packet_U8A, cSIZE_BUFFER_TX_MAX_U8-9);
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
		liste_de_paquets_a_envoyer_ENA[counter_packet_added_U8].TED_packet_UN.packet_U8A[index_U8] = TED_packet_to_send_UN.packet_U8A[index_U8];
	}
	return NRF_OK_EN;
}


TED_ret_val_en TED_ping_EN(uint8_t address_dst_U8)
{
	uint8_t payload[cSIZE_PAYLOAD_U8] = "Aurelie est si belle";
	return TED_packet_to_send_EN(address_dst_U8,PING,payload);
}

inline void print_rx_packet_with_string_payload(TED_packet_un TED_packet_UN)
{
	char string[cSIZE_BUFFER_TX_MAX_U8-12+1] = "";
	for (uint8_t index_U8=0 ; index_U8<cSIZE_BUFFER_TX_MAX_U8-12 ; index_U8++)
	{
		string[index_U8] = TED_packet_UN.packet_STR.payload_U8A[index_U8];
	}
	DBG_printString("\r\n",INFO_EN);
	DBG_printString("<Affichage du paquet recu>\r\n",INFO_EN);
	DBG_printString("Version du reseau TED24: ",INFO_EN);
	DBG_printUint32_t(TED_packet_UN.packet_STR.version_Ted24_U4,INFO_EN);
	DBG_printString("\r\n",INFO_EN);

	DBG_printString("ID du reseau TED24: ",INFO_EN);
	DBG_printUint32_t(TED_packet_UN.packet_STR.ID_reseau_U4,INFO_EN);
	DBG_printString("\r\n",INFO_EN);

	DBG_printString("Adresse emetteur: ",INFO_EN);
	DBG_printUint32_t(TED_packet_UN.packet_STR.address_emetteur_U8A[0],INFO_EN);
	DBG_printString("\r\n",INFO_EN);

	DBG_printString("Adresse destinataire: ",INFO_EN);
	DBG_printUint32_t(TED_packet_UN.packet_STR.address_Destinataire_U8,INFO_EN);
	DBG_printString("\r\n",INFO_EN);

	DBG_printString("Fonction: ",INFO_EN);
	DBG_printUint32_t(TED_packet_UN.packet_STR.function_U5,INFO_EN);
	DBG_printString("\r\n",INFO_EN);

	DBG_printString("Payload: ",INFO_EN);
	DBG_printString(string,INFO_EN);
	DBG_printString("\r\n",INFO_EN);

	DBG_printString("CRC recu: ",INFO_EN);
	DBG_printUint32_t(TED_packet_UN.packet_STR.crc8_Id_paquet_U8,INFO_EN);
	DBG_printString("\r\n",INFO_EN);

	TED_packet_UN.packet_STR.crc8_Id_paquet_U8=0;

	DBG_printString("CRC calcule: ",INFO_EN);
	DBG_printUint32_t(calculate_crc8_U8(TED_packet_UN.packet_U8A,cSIZE_BUFFER_TX_MAX_U8-9),INFO_EN);
	DBG_printString("\r\n",INFO_EN);
	DBG_printString("<Fin du paquet recu>\r\n",INFO_EN);
	DBG_printString("\r\n",INFO_EN);
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



inline TED_ret_val_en TED_ack_EN(uint8_t address_dst_U8, TED_function_en function_to_ack_EN, uint8_t Id_packet_to_ack)
{
	//TODO attention pas besoin de gere le mode Tx et Rx ici vu qu'on ne charge que le buffer d'envoi
	//TODO renvoyer dans le payload toutes les adresses par lesquelles le paquet a ack est passe
	TED_ret_val_en TED_ret_val_EN;
	NRF_ret_val_en NRF_ret_val_EN;
	uint8_t payload[cSIZE_PAYLOAD_U8] = {function_to_ack_EN,Id_packet_to_ack};

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
			NRF_ret_val_EN = NRF24_TxMode_EN((uint8_t *)PipeAddress, 10);
			if (NRF_ret_val_EN != NRF_OK_EN)
			{
				DBG_printString("<Switch to Tx mode impossible>\r\n", INFO_EN);
				//TODO à loguer comme une erreur et passer en mode erreur (ne rien faire sauf attendre une commande debug ?)
				tache_en_cours_EN = NO_TASK;
				return TED_TX_MODE_UNAVAILABLE_EN;
			}
			NRF_ret_val_EN = NRF24_Transmit_EN(liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].TED_packet_UN.packet_U8A,cSIZE_BUFFER_TX_MAX_U8); //envoie paquet TED24
			if (NRF_ret_val_EN != NRF_OK_EN)
			{
				NRF_ret_val_EN = NRF24_RxMode_EN((uint8_t *)PipeAddress, 10);
				if (NRF_ret_val_EN != NRF_OK_EN)
				{
					DBG_printString("<Switch to Rx mode impossible>\r\n", INFO_EN);
					//TODO à loguer comme une erreur et passer en mode erreur (ne rien faire sauf attendre une commande debug ?)
					tache_en_cours_EN = NO_TASK;
					return TED_RX_MODE_UNAVAILABLE_EN;
				}
				tache_en_cours_EN = NO_TASK;
				DBG_printString("<Packet not sent>\r\n", INFO_EN);
				return TED_SEND_PACKET_NOT_OK_EN;
			}
			else
			{
				NRF_ret_val_EN = NRF24_RxMode_EN((uint8_t *)PipeAddress, 10);
				if (NRF_ret_val_EN != NRF_OK_EN)
				{
					DBG_printString("<Switch to Rx mode impossible>\r\n", INFO_EN);
					//TODO à loguer comme une erreur et passer en mode erreur (ne rien faire sauf attendre une commande debug ?)
					tache_en_cours_EN = NO_TASK;
					return TED_RX_MODE_UNAVAILABLE_EN;
				}
				if (liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].TED_packet_UN.packet_STR.function_U5 != ACK)
				{
					liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].begin_waiting_ack_time_ms_U32 = HAL_millis_U32();
					liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].flag_is_waiting_for_ack = true;
					tache_en_cours_EN = WAITING_FOR_ACK_TASK;
				}
				else
				{    // le paquet d'ack n'attend pas d'ack
					liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].begin_waiting_ack_time_ms_U32 = 0;
					liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].flag_is_waiting_for_ack = false;
					tache_en_cours_EN = NO_TASK;
				}
				DBG_printString("<Waiting for ack...", INFO_EN);
				return TED_OK_EN;
			}
			break;
		case WAITING_FOR_ACK_TASK:
			if (liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].is_ack_rx == true) //ACK recu
			{
				DBG_printString("<ACK received>\r\n", INFO_EN);
				tache_en_cours_EN = NO_TASK;
				liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].time_ack_is_received_ms_U32 = HAL_millis_U32()-liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].begin_waiting_ack_time_ms_U32;
				return TED_ACK_IS_RECEIVED;
			}
			else if (HAL_millis_U32()-liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].begin_waiting_ack_time_ms_U32 > cWAITING_TIMEOUT_ACK_ms_U32)
			{
				DBG_printString(">\r\n", INFO_EN);
				DBG_printString("<ACK not received>\r\n", INFO_EN);
				tache_en_cours_EN = NO_TASK;
				liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].flag_is_waiting_for_ack = false;
				liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].time_ack_is_received_ms_U32 = HAL_millis_U32()-liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].begin_waiting_ack_time_ms_U32;
				counter_packet_error_U8++;
				return TED_TIMEOUT_ACK_IS_NOT_RECEIVED;
			}
			else
			{
				//on attend toujours l'ACK et on est pas en timeout
				DBG_printString(".", INFO_EN);
				return TED_WAITING_FOR_ACK;
			}
			break;
		case NO_TASK:
			return TED_NO_TASK_RUNNING_EN;
			break;
		default:
			return TED_WRONG_TASK_EN;
			break;

	}
}

TED_ret_val_en IsPacketForMe_B (TED_packet_un TED_packet_UN)
{
	if (TED_packet_UN.packet_STR.ID_reseau_U4 != local_TED_config_node_TED.network_ID_U4)
	{
		return TED_WRONG_NETWORK_ID_EN;
	}
	else if (TED_packet_UN.packet_STR.address_Destinataire_U8 != local_TED_config_node_TED.my_address_U8)
	{
		return TED_WRONG_DST_ADR_EN;
	}
	else if (TED_packet_UN.packet_STR.crc8_Id_paquet_U8 != calculate_crc8_U8(TED_packet_UN.packet_U8A,cSIZE_BUFFER_TX_MAX_U8-9))
	{
		return TED_WRONG_CRC_EN;
	}
	else if (TED_packet_UN.packet_STR.version_Ted24_U4 != local_TED_config_node_TED.protocol_version_U4)
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
	if (TED_IsDataAvailable_B())//on a recu un paquet, il faut le stocker dans la stack
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
	else if (counter_packet_received_U8 != counter_packet_treated_U8) //Il y a un paquet a traiter
	{
		counter_packet_treated_U8++;
		TED_treatRxPacket(liste_de_paquets_recus_ENA[counter_packet_treated_U8].TED_packet_UN);//TODO traiter la valeur de retour ?
		//TODO gerer le temps de reception du ACK (timeout ?)
	}
	else
	{
		//TODO gerer ce cas
		return;
	}

}

void Ted_Process(void)
{
	TED_processRxPacket();
	TED_processTxPacket();
}


static TED_ret_val_en TED_treatRxPacket(TED_packet_un TED_packet_UN)
{
	switch(TED_packet_UN.packet_STR.function_U5)
	{
		case PING:
			DBG_printString("<Ping packet received>\r\n",INFO_EN);
			TED_ack_EN(TED_packet_UN.packet_STR.address_emetteur_U8A[0], PING,TED_packet_UN.packet_STR.crc8_Id_paquet_U8);
			break;

		case ACK:
			DBG_printString("<ACK packet received ",INFO_EN);
			if (liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].flag_is_waiting_for_ack == true && //si le paquet qui a ete envoye attend un ack
				TED_packet_UN.packet_STR.payload_U8A[1] == liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].TED_packet_UN.packet_STR.crc8_Id_paquet_U8 &&
				TED_packet_UN.packet_STR.payload_U8A[0] == liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].TED_packet_UN.packet_STR.function_U5)
			{
				DBG_printString("and corresponding to the actual Tx packet>\r\n",INFO_EN);
				liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].flag_is_waiting_for_ack = false;
				liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].is_ack_rx = true;
				liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].time_ack_is_received_ms_U32 = HAL_millis_U32()-liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].begin_waiting_ack_time_ms_U32;
				DBG_printString("<ACK received ",INFO_EN);
				DBG_printUint32_t(liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].time_ack_is_received_ms_U32,INFO_EN);
				DBG_printString("ms after sending the packet >\r\n",INFO_EN);
				return NRF_OK_EN;
			}
			else
			{
				DBG_printString("but not corresponding to the actual Tx packet>\r\n",INFO_EN);
				counter_packet_error_U8++;
				return TED_ACK_RECEIVED_NOT_CORRESPONDING_TO_ACTUAL_TX_PACKET;
			}

			break;
		default:
			DBG_printString("<Packet received but function not found>\r\n",INFO_EN);
			//TODO afficher en debug que la fonction n'a pas ete reconnue
			return TED_COMMAND_NOT_FOUND_EN;
	}
	return TED_OK_EN;
}



