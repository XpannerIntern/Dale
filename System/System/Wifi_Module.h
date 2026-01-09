/*
 * Wifi_Module.h
 *
 * Created: 2026-01-09 오전 9:39:50
 *  Author: User
 */ 


#ifndef WIFI_MODULE_H_
#define WIFI_MODULE_H_

/*#define F_CPU 16000000UL*/

#include <stdint.h>

#define DN_BAUD_RATE 9600
#define DN_UBRR (F_CPU/8/DN_BAUD_RATE-1)

extern const char* SSID;
extern const char* PASS;
extern const char* LAPTOP_IP;

void LED_Toggle(void);

void USART_Init(unsigned int);

void USART_Transmit(unsigned char);

void USART_SendString(const char*);

unsigned char USART_Receive(void);

void USART_FlushRx(void);

uint8_t Wait_OK(uint16_t);

void LED_Toggle_D13(void);

void LED_Blink3_Fail(void);

uint8_t  JoinAP(void);

uint8_t Send_Intrusion(void);

void start_wifi(void);



#endif /* WIFI_MODULE_H_ */