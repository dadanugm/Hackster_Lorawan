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
void module_sleep(void);

//variable lora node for gps
uint8_t dev_eui[8] ={0x00,0x00,0x00,0x00,0x03,0x57,0x35,0x51};
uint8_t app_eui[8] ={0x00,0x00,0x00,0x00,0x03,0x57,0x35,0x10};
uint8_t app_key[16] ={0x2b,0x7e,0x15,0x16,0x28,0xae,0xd2,0xa6,0xab,0xf7,0x15,0x88,0x09,0xcf,0x4f,0x3c};
// variable for gps
uint8_t latitude[10];
uint8_t lat_pole[2];
uint8_t longitude[11];
uint8_t long_pole[2];
uint8_t altitude[8];
uint8_t alt_unit[2];
uint8_t lora_payload[70];
int payload_len;
int ready = 0;
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
				rw_LoRaTxData(0,2,payload_len,lora_payload);	
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
	int i;
	for(i=0;i<sizeof(latitude);i++){
		latitude[i] = NmeaGpsData.NmeaLatitude[i];}
	
	for(i=0;i<sizeof(longitude);i++){
		longitude[i] = NmeaGpsData.NmeaLongitude[i];}
	
	for(i=0;i<2;i++){
		lat_pole[i] = NmeaGpsData.NmeaLatitudePole[i];
		long_pole[i] = NmeaGpsData.NmeaLongitudePole[i];
		alt_unit[i] = NmeaGpsData.NmeaAltitudeUnit[i];}
	
	for(i=0;i<sizeof(altitude);i++){
		altitude[i] = NmeaGpsData.NmeaAltitude[i];}
	
	payload_len = sprintf((char*)lora_payload,"%i%i%i%i%i%i%i%i%i%i,%i%i,%i%i%i%i%i%i%i%i%i%i%i,%i%i,%i%i%i%i%i%i%i%i,%i%i",
		latitude[0],latitude[1],latitude[2],latitude[3],latitude[4],latitude[5],latitude[6],
		latitude[7],latitude[8],latitude[9],lat_pole[0],lat_pole[1],
		longitude[0],longitude[1],longitude[2],longitude[3],longitude[4],longitude[5],longitude[6],
		longitude[7],longitude[8],longitude[9],longitude[10],long_pole[0],long_pole[1],
		altitude[0],altitude[1],altitude[2],altitude[3],altitude[4],altitude[5],altitude[6],
		altitude[7],alt_unit[0],alt_unit[1]);
	
	e_printf(lora_payload);
}


void module_sleep(void)
{
	DelayMs(10);
  
  SX1276SetSleep();
  SX1276Write(REG_OPMODE,SX1276Read(REG_OPMODE)& 0xF8);
    
  __HAL_RCC_RTC_DISABLE();
  
  __HAL_RCC_LSE_CONFIG(RCC_LSE_OFF);

    
  BoardDeInitMcu();
  SysEnterUltraPowerStopMode();
  BoardInitMcu();
  
  __HAL_RCC_LSE_CONFIG(RCC_LSE_ON);
  
  __HAL_RCC_RTC_ENABLE();

  SX127X_INIT();
	
	GPIOIRQ_Enable();
  
  rw_LoadUsrConfig(); 
  
  lora_recv(LORA_EVENT_WAKEUP, 0, 0, NULL);
  
  //UartFlush(&Uart1);
}


