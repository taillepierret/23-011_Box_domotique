/*
 * Ted24.c
 *
 *  Created on: Oct 14, 2023
 *      Author: teddy
 */

//TODO etudier pour le bootloader
//TODO faire la partie mesh du réseau
//TODO historiser les problemes rencontrés
//TODO faire un compteur de paquets supprimés
//TODO enregistrer tous les logs dans l'EEPROM et afficher une led rouge quadn l'EEPROM est full, faire une commande pour envoyer les logs en debug et en faire une autre pour tout flusher
//TODO faire en sorte que chaque fonction qui stocke un paquet dans la pile d'envoie renvoie l'ID du paquet a envoyer pour pouvoir faire une focntion qui controle si le packet en question a été ack
//TODO faire une fonction qui permet d'ajouter une command/function en plus avec le pointeur de focntion a appeler si l'on recoit ce code pour la fonction de traitement des paquets recus

#include "Ted24.h"
#include "tools.h"
#include "debug.h"



//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------- local structures -----------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
typedef struct{
	uint8_t my_address_U8;
	bool flag_activating_low_power_mode_B;
	bool flag_node_is_init_B;
	uint8_t network_ID_U4:4;
	uint8_t protocol_version_U4:4;
}TED_config_node_str;

typedef struct{
	TED_packet_un TED_packet_UN;
	//uint32_t received_time_ms_U32;
}TED_Rx_packet_STR;

typedef struct{
	TED_packet_un TED_packet_UN;
	bool flag_is_waiting_for_ack;
	uint32_t begin_waiting_ack_time_ms_U32; //correspond au temps ou le paquet a ete envoye
	//uint16_t time_ack_is_received_ms_U32;  //correspond au temps ou le ACK est recu
	bool is_ack_rx;
	uint8_t counter_retry_sending_U8;
}TED_Tx_packet_STR;

typedef struct{
	TED_ret_val_en TED_error_code_EN;
	uint8_t localisation_code_U8;
	//TODO ajouter l'information de temps
	//faire une fonction de get pour le tableau de ces erreur
	//faire une fonction de flush
	//faire une focntion de append
}TED_error_code_str;

//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------- Local constants ------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
#define cSIZE_STACK_PACKET_TO_SEND_U8        ((uint8_t)  20) //la vraie valeur est celle-ci -1
#define cSIZE_STACK_PACKET_RECEIVED_U8       ((uint8_t)  20)
#define cSIZE_STACK_ERROR_U8                 ((uint8_t)  20)
#define cWAITING_TIMEOUT_ACK_ms_U32          ((uint32_t) 5000)
#define cRETRY_SENDING_PACKET_NUMBER_MAX_U8  ((uint8_t)  3)

static const uint8_t PipeAddress[] = {0xEE,0xDD,0xCC,0xBB,0xAA};

//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------- Local variables ------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------

static TED_config_node_str local_TED_config_node_TED = {.my_address_U8 = 0,
														.flag_activating_low_power_mode_B = false,
														.flag_node_is_init_B = false,
														.network_ID_U4 = 0,
														.protocol_version_U4 = cPROTOCOL_VERSION_U8};

static uint8_t counter_packet_sended_U8 = 0;
static uint8_t counter_packet_added_U8 = 0;

static TED_Rx_packet_STR liste_de_paquets_recus_ENA[cSIZE_STACK_PACKET_RECEIVED_U8] = {0};
static TED_Tx_packet_STR liste_de_paquets_a_envoyer_ENA[cSIZE_STACK_PACKET_TO_SEND_U8] = {0};

static uint8_t counter_packet_received_U8 = 0;
static uint8_t counter_packet_treated_U8 = 0;

static TED_task_en tache_en_cours_EN = NO_TASK;

static uint8_t counter_packet_error_U8 = 0;

//static TED_error_code_str stack_error_STRA[cSIZE_STACK_ERROR_U8] = {0};


//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------- Local functions prototypes -------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
static TED_ret_val_en TED_packet_to_send_EN(uint8_t addr_dst_U8, TED_function_en function_EN, uint8_t payload_U8A[cSIZE_PAYLOAD_U8]);
static TED_ret_val_en TED_treatRxPacket(TED_packet_un TED_packet_UN);
static void TED_processRxPacket (void);
static TED_ret_val_en IsPacketForMe_B (TED_packet_un TED_packet_UN);
static TED_ret_val_en TED_processTxPacket (void);
static TED_ret_val_en TED_sendAck_EN(TED_packet_un TED_packet_UN);
static TED_ret_val_en TED_readReceivePacket_EN(TED_packet_un* TED_packet_UN);
static void print_packet_with_string_payload(TED_packet_un TED_packet_UN);
static TED_ret_val_en IsPacketHasToBeResentForMesh_B (TED_packet_un TED_packet_UN);


//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------- Local functions ------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------
/**
 * @fn void print_packet_with_string_payload(TED_packet_un)
 * @brief This function print details of the packet in parameters with the payload in string
 *
 * @param TED_packet_UN, the packet to print
 */
static inline void print_packet_with_string_payload(TED_packet_un TED_packet_UN)
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

	TED_packet_UN.packet_STR.crc8_Id_paquet_U8=0; //TODO a retirer ?

	DBG_printString("CRC calcule: ",INFO_EN);
	DBG_printUint32_t(calculate_crc8_U8(TED_packet_UN.packet_U8A,cSIZE_BUFFER_TX_MAX_U8-9),INFO_EN);
	DBG_printString("\r\n",INFO_EN);
	DBG_printString("<Fin du paquet recu>\r\n",INFO_EN);
	DBG_printString("\r\n",INFO_EN);
}

/**
 * @fn TED_ret_val_en TED_readReceivePacket_EN(TED_packet_un*)
 * @brief Stores in the parameter the received packet, to know if there is a packet to read: call TED_IsDataAvailable_B
 *
 * @param TED_packet_UN the variable to store the reading packet
 * @return error code:TED_OK_EN if no issue or TED_READING_PACKET_NOT_POSSIBLE_EN if there is an issue in the reading action on the nrf24l01
 */
static inline TED_ret_val_en TED_readReceivePacket_EN(TED_packet_un* TED_packet_UN)
{
	NRF_ret_val_en NRF_ret_val_EN;
	NRF_ret_val_EN = NRF24_Receive_EN(TED_packet_UN->packet_U8A);
	if (NRF_ret_val_EN != NRF_OK_EN)
	{
		return TED_READING_PACKET_NOT_POSSIBLE_EN;
	}
	else
	{
		return TED_OK_EN;
	}
}

/**
 * @fn TED_ret_val_en TED_sendAck_EN(uint8_t, TED_function_en, uint8_t)
 * @brief stores in the sending stack an ACK. All the commands except this one are acknoledgable. To ack a command, we need to know the destination address,
 * the function we want to ack and the ID/CRC of the packet we want to ack that is why the packet to ack is in parameters. we load in the ack payload in [0]
 * the fucntion to ack and in [1] the ID/CRC
 *
 * @param address_dst_U8
 * @param function_to_ack_EN
 * @param Id_packet_to_ack
 * @return TED_OK_EN, TED_SENDING_STACK_OVERFLOW_EN
 */
static inline TED_ret_val_en TED_sendAck_EN(TED_packet_un TED_packet_UN)
{
	uint8_t payload[cSIZE_PAYLOAD_U8] = {TED_packet_UN.packet_STR.function_U5,TED_packet_UN.packet_STR.crc8_Id_paquet_U8};

	for (uint8_t index_U8=0 ; index_U8<8 ; index_U8++)
	{
		payload[2+index_U8] = TED_packet_UN.packet_STR.address_emetteur_U8A[index_U8];
	}

	return TED_packet_to_send_EN(TED_packet_UN.packet_STR.address_emetteur_U8A[0],ACK,payload);
}

/**
 * @fn TED_ret_val_en TED_processTxPacket(void)
 * @brief transmit process management. read the sending stack and send to the nrf24l01 the packet to send then wait for the ack. If no ack received, retry.
 * During this no other packet can be sent.
 *
 * @return TED_TX_MODE_UNAVAILABLE_EN, TED_RX_MODE_UNAVAILABLE_EN, TED_SEND_PACKET_NOT_OK_EN, TED_OK_EN, TED_ACK_IS_RECEIVED, TED_TIMEOUT_ACK_IS_NOT_RECEIVED, TED_WAITING_FOR_ACK,
 * TED_NUMBER_OF_RETRY_TOO_BIG_EN, TED_RETRY_EN, TED_NO_TASK_RUNNING_EN, TED_WRONG_TASK_EN
 */
static TED_ret_val_en TED_processTxPacket (void)
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
				//stack_error_STRA.TED_error_code_EN = NRF_ret_val_EN;
				//stack_error_STRA.localisation_code_U8 = 2;
				tache_en_cours_EN = NO_TASK;
				return TED_TX_MODE_UNAVAILABLE_EN;
			}
			NRF_ret_val_EN = NRF24_Transmit_EN(liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].TED_packet_UN.packet_U8A,cSIZE_BUFFER_TX_MAX_U8); //envoie paquet TED24
			if (NRF_ret_val_EN != NRF_OK_EN)
			{
				//stack_error_STRA.TED_error_code_EN = NRF_ret_val_EN;
				//stack_error_STRA.localisation_code_U8 = 2;
				NRF_ret_val_EN = NRF24_RxMode_EN((uint8_t *)PipeAddress, 10);
				if (NRF_ret_val_EN != NRF_OK_EN)
				{
					//stack_error_STRA.TED_error_code_EN = NRF_ret_val_EN;
					//stack_error_STRA.localisation_code_U8 = 2;
					DBG_printString("<Switch to Rx mode impossible>\r\n", INFO_EN);
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
					//stack_error_STRA.TED_error_code_EN = NRF_ret_val_EN;
					//stack_error_STRA.localisation_code_U8 = 3;
					DBG_printString("<Switch to Rx mode impossible>\r\n", INFO_EN);
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
				return TED_SEND_PACKET_OK_EN;
			}
			break;
		case WAITING_FOR_ACK_TASK:
			if (liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].is_ack_rx == true) //ACK recu
			{
				tache_en_cours_EN = NO_TASK;
				//liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].time_ack_is_received_ms_U32 = HAL_millis_U32()-liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].begin_waiting_ack_time_ms_U32;
				return TED_ACK_IS_RECEIVED;
			}
			else if (HAL_millis_U32()-liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].begin_waiting_ack_time_ms_U32 > cWAITING_TIMEOUT_ACK_ms_U32) //Timeout, ack non recu
			{
				DBG_printString(">\r\n", INFO_EN);
				DBG_printString("<ACK not received>\r\n", INFO_EN);
				//stack_error_STRA.TED_error_code_EN = TED_TIMEOUT_ACK_IS_NOT_RECEIVED;
				//stack_error_STRA.localisation_code_U8 = 2;
				tache_en_cours_EN = RETRY_SENDING_TASK;
				liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].flag_is_waiting_for_ack = false;
				//liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].time_ack_is_received_ms_U32 = HAL_millis_U32()-liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].begin_waiting_ack_time_ms_U32;
				return TED_TIMEOUT_ACK_IS_NOT_RECEIVED;
			}
			else
			{
				//on attend toujours l'ACK et on est pas en timeout
				DBG_printString(".", INFO_EN);
				return TED_WAITING_FOR_ACK;
			}
			break;
		case RETRY_SENDING_TASK:
			liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].counter_retry_sending_U8++;
			if(liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].counter_retry_sending_U8 > cRETRY_SENDING_PACKET_NUMBER_MAX_U8)
			{
				tache_en_cours_EN = NO_TASK;
				counter_packet_error_U8++;
				//stack_error_STRA.TED_error_code_EN = TED_NUMBER_OF_RETRY_TOO_BIG_EN;
				//stack_error_STRA.localisation_code_U8 = 2;
				return TED_NUMBER_OF_RETRY_TOO_BIG_EN;
			}
			else
			{
				tache_en_cours_EN = SENDING_PACKET_TASK;
				DBG_printString("---------------------------------------------------------------------------------------------------------------------\r\n", INFO_EN);
				DBG_printString("<Retry number: ", INFO_EN);
				DBG_printUint32_t(liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].counter_retry_sending_U8, INFO_EN);
				DBG_printString(">\r\n", INFO_EN);
				if(counter_packet_sended_U8==0)
				{
					counter_packet_sended_U8 = cSIZE_STACK_PACKET_TO_SEND_U8-1;
				}
				else
				{
					counter_packet_sended_U8--;
				}
				return TED_RETRY_EN;
			}
		case NO_TASK:
			return TED_NO_TASK_RUNNING_EN;
			break;
		default:
			//stack_error_STRA.TED_error_code_EN = TED_WRONG_TASK_EN;
			//stack_error_STRA.localisation_code_U8 = 2;
			return TED_WRONG_TASK_EN;
			break;

	}
}

/**
 * @fn TED_ret_val_en IsPacketForMe_B(TED_packet_un)
 * @brief check if the packet in parameter is addressed to this node.
 *
 * @param TED_packet_UN
 * @return TED_WRONG_NETWORK_ID_EN, TED_WRONG_DST_ADR_EN, TED_WRONG_CRC_EN, TED_WRONG_PROTOCOL_VERSION_EN or TED_OK_EN
 */
static TED_ret_val_en IsPacketForMe_B (TED_packet_un TED_packet_UN)
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
	//TODO verifier si le paquet a deja ete recu dans un lapse de temps court, ajouter la date et l'heure de reception des paquets
	/*for (uint8_t index_U8=0 ; index_U8<cSIZE_STACK_PACKET_RECEIVED_U8 ; index_U8++)
	{
		if (TED_packet_UN.packet_STR.crc8_Id_paquet_U8)
	}*/
	return TED_OK_EN;
}


static TED_ret_val_en IsPacketHasToBeResentForMesh_B (TED_packet_un TED_packet_UN)
{
	if (TED_packet_UN.packet_STR.ID_reseau_U4 != local_TED_config_node_TED.network_ID_U4)
	{
		return TED_WRONG_NETWORK_ID_EN;
	}
	else if (TED_packet_UN.packet_STR.crc8_Id_paquet_U8 != calculate_crc8_U8(TED_packet_UN.packet_U8A,cSIZE_BUFFER_TX_MAX_U8-9))
	{
		return TED_WRONG_CRC_EN;
	}
	else if (TED_packet_UN.packet_STR.version_Ted24_U4 != local_TED_config_node_TED.protocol_version_U4)
	{
		return TED_WRONG_PROTOCOL_VERSION_EN;
	}
	//TODO verifier si le paquet a deja ete recu dans un lapse de temps court, ajouter la date et l'heure de reception des paquets
	/*for (uint8_t index_U8=0 ; index_U8<cSIZE_STACK_PACKET_RECEIVED_U8 ; index_U8++)
	{
		if (TED_packet_UN.packet_STR.crc8_Id_paquet_U8)
	}*/
	return TED_OK_EN;
}

/**
 * @fn void TED_processRxPacket(void)
 * @brief Receive process management. Check if a packet is waiting for reading and if the packet is addressed to this node, if so it is stored in the receive packet stack. And if there is a
 * packet to treat in the receive packet stack. In the future, the mesh trasnmit packet will be treat here.
 *  stack_error_STRA
 */
static void TED_processRxPacket (void)
{
	TED_packet_un TED_Rx_packet_UN;
	TED_ret_val_en TED_ret_val_EN;
	if (TED_IsDataAvailable_B())//on a recu un paquet, il faut le stocker dans la stack
	{
		TED_ret_val_EN = TED_readReceivePacket_EN(&TED_Rx_packet_UN);
		if (TED_ret_val_EN != TED_OK_EN)
		{
			//stack_error_STRA.TED_error_code_EN = TED_ret_val_EN;
			//stack_error_STRA.localisation_code_U8 = 0;
		}
		print_packet_with_string_payload(TED_Rx_packet_UN);
		if(IsPacketForMe_B(TED_Rx_packet_UN) == TED_OK_EN)
		{
			counter_packet_received_U8++;
			if (counter_packet_received_U8>=cSIZE_STACK_PACKET_RECEIVED_U8)
			{
				counter_packet_received_U8 = 0;
			}
			//TODO loguer si jamais on ecrit sur un paquet non traité
			for (uint8_t index_U8=0 ; index_U8<cSIZE_BUFFER_TX_MAX_U8 ; index_U8++)
			{
				liste_de_paquets_recus_ENA[counter_packet_received_U8].TED_packet_UN.packet_U8A[index_U8] = TED_Rx_packet_UN.packet_U8A[index_U8];
			}
			//liste_de_paquets_recus_ENA[counter_packet_received_U8].received_time_ms_U32 = HAL_millis_U32();
		}
		//TODO else if c'est un paquet a renvoyer: le stocker dans la pile de paquet a renvoyer et attendre si l'on recoit l'ACK, si le paquet est un ack, il faut l'envoyer. Si on a pas recu l'ack au bout d'un temps random alors il faut l'envoyer sinon on ne l'envoie pas
		//IsPacketHasToBeResentForMesh_B
	}
	else if (counter_packet_received_U8 != counter_packet_treated_U8) //Il y a un paquet a traiter
	{
		counter_packet_treated_U8++;
		if (counter_packet_treated_U8>=cSIZE_STACK_PACKET_RECEIVED_U8)
		{
			counter_packet_treated_U8 = 0;
		}
		TED_ret_val_EN = TED_treatRxPacket(liste_de_paquets_recus_ENA[counter_packet_treated_U8].TED_packet_UN);
		if (TED_ret_val_EN != TED_OK_EN)
		{
			//stack_error_STRA.TED_error_code_EN = TED_ret_val_EN;
			//stack_error_STRA.localisation_code_U8 = 1;
			return;
		}
	}
	else
	{
		//TODO gerer ce cas
		return;
	}

}

/**
 * @fn TED_ret_val_en TED_treatRxPacket(TED_packet_un)
 * @brief permit to do an action in function of the code function received
 *
 * @param TED_packet_UN the received packet
 * @return TED_ACK_RECEIVED_NOT_CORRESPONDING_TO_ACTUAL_TX_PACKET, TED_COMMAND_NOT_FOUND_EN, NRF_OK_EN
 */
static TED_ret_val_en TED_treatRxPacket(TED_packet_un TED_packet_UN)
{
	if(local_TED_config_node_TED.flag_node_is_init_B != true)
	{
		DBG_printString("TED24 protocol not init...", ERROR_EN);
		while(1)
		{
			DBG_printString(".", ERROR_EN);
		}
	}
	switch(TED_packet_UN.packet_STR.function_U5)
	{
		case PING:
			DBG_printString("<Ping packet received>\r\n",INFO_EN);
			TED_sendAck_EN(TED_packet_UN);//TODO gerer la valeur de retour
			return NRF_OK_EN;
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
				//liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].time_ack_is_received_ms_U32 = HAL_millis_U32()-liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].begin_waiting_ack_time_ms_U32;
				DBG_printString("<ACK received ",INFO_EN);
				//DBG_printUint32_t(liste_de_paquets_a_envoyer_ENA[counter_packet_sended_U8].time_ack_is_received_ms_U32,INFO_EN);
				DBG_printString("ms after sending the packet>\r\n\r\n",INFO_EN);
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
			return TED_COMMAND_NOT_FOUND_EN;
	}
}

/**
 * @fn TED_ret_val_en TED_packet_to_send_EN(uint8_t, TED_function_en, uint8_t[])
 * @brief store in the transmit packet stack a packet to send
 *
 * @param addr_dst_U8
 * @param function_EN
 * @param payload_U8A
 * @return NRF_OK_EN, TED_SENDING_STACK_OVERFLOW_EN
 */
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
		//TODO gerer le cas ou l'on s'apprette a ecrire sur un paquet non envoye: ajouter au logs
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



//----------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------- Global functions -----------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------------

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
	if(local_TED_config_node_TED.flag_node_is_init_B != true)
	{
		DBG_printString("TED24 protocol not init...", ERROR_EN);
		while(1)
		{
			DBG_printString(".", ERROR_EN);
		}
	}
	return NRF24_isDataAvailable_EN(1) == NRF_DATA_AVAILABLE_EN;
}


inline TED_ret_val_en TED_ping_EN(uint8_t address_dst_U8)
{
	if(local_TED_config_node_TED.flag_node_is_init_B != true)
	{
		DBG_printString("TED24 protocol not init...", ERROR_EN);
		while(1)
		{
			DBG_printString(".", ERROR_EN);
		}
	}
	uint8_t payload[cSIZE_PAYLOAD_U8] = "Aurelie est si belle";
	return TED_packet_to_send_EN(address_dst_U8,PING,payload);
}

inline void Ted_Process(void)
{
	if(local_TED_config_node_TED.flag_node_is_init_B != true)
	{
		DBG_printString("TED24 protocol not init...", ERROR_EN);
		while(1)
		{
			DBG_printString(".", ERROR_EN);
		}
	}

	TED_processRxPacket();
	TED_processTxPacket();
}

inline uint8_t TED_getCounterErrorValue_U8(void)
{
	return counter_packet_error_U8;
}

inline void TED_printCounterErrorValue(void)
{
	DBG_printString("<COUNTER ERROR VALUE: ", ERROR_EN);
	DBG_printUint32_t(counter_packet_error_U8, ERROR_EN);
	DBG_printString(">\r\n", ERROR_EN);
}

