/*
/ _____)             _              | |
( (____  _____ ____ _| |_ _____  ____| |__
\____ \| ___ |    (_   _) ___ |/ ___)  _ \
_____) ) ____| | | || |_| ____( (___| | | |
(______/|_____)_|_|_| \__)_____)\____)_| |_|
(C)2013 Semtech

Description: LoRaMac classA device implementation

License: Revised BSD License, see LICENSE.TXT file include in the project

Maintainer: Miguel Luis and Gregory Cristian
*/

//#define  LORA_HF_BOARD

/*! \file classA/LoRaMote/main.c */
#include <time.h>
#include <string.h>
#include "board.h"

#include "app.h"
#include "rw_lora.h"
#include "rw_sys.h"

lora_config_t g_lora_config;
extern void lora_cli_loop(void);
// object
TimerEvent_t join_timer;
// functions
void timer_join_callback(void);
void get_gps_data(void);

//variable lora node for Ultrasonic
uint8_t dev_eui[8] ={0x00,0x00,0x00,0x00,0x03,0x57,0x35,0x51};
uint8_t app_eui[8] ={0x00,0x00,0x00,0x00,0x03,0x57,0x35,0x10};
uint8_t app_key[16] ={0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c};
// variable for gps
uint8_t latitude[10];
uint8_t longitude[11];


// variable
int ready = 0;
extern uint8_t lora_data_send[128];
extern uint8_t lora_data_length;
uint8_t tes[4] = "test";


int main( void )
{
    uart_config_t uart_config;
    BoardInitMcu( );
    
    if (read_partition(PARTITION_1, (char *)&uart_config, sizeof(uart_config)) < 0) {
        SET_UART_CONFIG_DEFAULT(uart_config);
    } 
    
    UartMcuInit(&Uart1, 1, UART_TX, UART_RX);
    UartMcuConfig(&Uart1, RX_TX, 9600, uart_config.wordLength, uart_config.stopBits, uart_config.parity, uart_config.flowCtrl);									
    e_printf("Welcome to RAK811.\r\n");		
    rw_ReadUsrConfig();	
    rw_InitLoRaWAN();
		
		g_lora_config.lora_mode = 0;
		g_lora_config.loraWan_class = 0;
		g_lora_config.def_tx_dr = 0;
		for (int i=0;i<8;i++){g_lora_config.dev_eui[i] = dev_eui[i];g_lora_config.app_eui[i] = app_eui[i];}
		for (int i=0;i<16;i++){g_lora_config.app_key[i] = app_key[i];}

    rw_LoadUsrConfig();
		GPIOIRQ_Enable();	
		GpsInit();
		TimerInit(&join_timer,timer_join_callback);
		TimerSetValue(&join_timer,180000);
		TimerStart(&join_timer);

		rw_JoinNetworkOTAA(g_lora_config.dev_eui,g_lora_config.app_eui,g_lora_config.app_key,31);
		
#if 0
    DelayMs(5000);
    enter_sleep();
#endif
    e_printf("Initialization OK!\r\n");
    while(1) {
			
			if (ready == 1)
			{
				get_gps_data();
				ready = 0;
				rw_LoRaTxData(0,2,4,tes);	
			}
        lora_cli_loop();
        TimerLowPowerHandler( );
    }
}

void timer_join_callback(void)
{
	ready = 1;
	TimerStop(&join_timer);
	TimerSetValue(&join_timer,180000);
	TimerStart(&join_timer);	
}

void get_gps_data(void)
{
	for(int i=0;i<sizeof(latitude);i++)
	{
		latitude[i] = NmeaGpsData.NmeaLatitude[i];
		e_printf_raw(latitude,sizeof(latitude));
	}
	
	for(int j=0;j<sizeof(longitude);j++)
	{
		longitude[j] = NmeaGpsData.NmeaLongitude[j];
		e_printf_raw(longitude,sizeof(longitude));
	}
}


