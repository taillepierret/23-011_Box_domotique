#include "../Inc/Application.h"
#include "../../Plateforme-RF24/Firmware_24-006_plateforme_RF24/Inc/log.h"
#include "../Inc/configuration.h"
#include "../Inc/hal.h"


LOG_HAL_functions_str LOG_HAL_functions_STR =
{
	.InitDebugUart = &HAL_InitDebugUart,
	.HAL_GetTime = &HAL_GetTime,
	.HAL_DebugPrint = &HAL_print_string
};

void runApp(void)
{
	//initialisation du hardware
	LOG_Init(&LOG_HAL_functions_STR);





	LOG_setLogLevel(LOG_LEVEL_VERBOSE_EN);
	LOG_PrintStringCRLF("<PROGARMME RUNNING>", LOG_SHOW_TIME_B, LOG_LEVEL_INFO_EN, LOG_SHOW_LOG_LEVEL_B);

	//affichage du type de produit
	LOG_PrintString("Type de produit: ", LOG_HIDE_TIME_B, LOG_LEVEL_INFO_EN, LOG_SHOW_LOG_LEVEL_B);
	LOG_PrintStringCRLF(PRODUCT_TYPE_CA, LOG_HIDE_TIME_B, LOG_LEVEL_INFO_EN, LOG_HIDE_LOG_LEVEL_B);

	//affichage de la version du firmware
	LOG_PrintString("Firmware version: ", LOG_HIDE_TIME_B, LOG_SHOW_LOG_LEVEL_B, LOG_SHOW_LOG_LEVEL_B);
	LOG_PrintStringCRLF(VERSION_FIRMWARE_CA, LOG_HIDE_TIME_B, LOG_HIDE_LOG_LEVEL_B, LOG_HIDE_LOG_LEVEL_B);
	
	//affichage de l'ID reseau
	LOG_PrintString("ID reseau: ", LOG_HIDE_TIME_B, LOG_SHOW_LOG_LEVEL_B, LOG_SHOW_LOG_LEVEL_B);
	LOG_PrintUint8CRLF(NETWORK_ID_U8, LOG_LEVEL_INFO_EN);



	while(1)
	{
		
	}
}


