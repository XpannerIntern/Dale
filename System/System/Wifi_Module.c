/*
 * Wifi_Module.c
 *
 * Created: 2026-01-09 오전 9:39:32
 *  Author: User
 */ 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "Wifi_Module.h"

const char* SSID = "dn";
const char* PASS = "godwjdqn1Q@";
const char* LAPTOP_IP = "10.222.24.249";

void LED_Toggle(void)
{
	DDRB |= 0x05;
	/* Replace with your application code */
	while (1)
	{
		PORTB |= 0x05;
		_delay_ms(500);
		
		PORTB &= ~0x05;
		_delay_ms(500);
	}
}

void USART_Init(unsigned int ubrr)
{
	
	UCSR0A = (1<<U2X0);                  // Double Speed
	UBRR0H = (uint8_t)(ubrr >> 8);
	UBRR0L = (uint8_t)(ubrr);

	UCSR0B = (1<<RXEN0) | (1<<TXEN0);
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);

}

void USART_Transmit(unsigned char data)
{
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = data;
}

void USART_SendString(const char* str)
{
	while(*str) USART_Transmit(*str++);
}

unsigned char USART_Receive(void)
{
	while (!(UCSR0A & (1<<RXC0)));
	return UDR0;
}

void USART_FlushRx(void)
{
	while (UCSR0A & (1<<RXC0)) (void)UDR0;
}

uint8_t Wait_OK(uint16_t timeout_ms)
{
	uint8_t seenO = 0;
	while (timeout_ms--)
	{
		while (UCSR0A & (1<<RXC0))
		{
			char c = UDR0;
			if (seenO && c == 'K') return 1;
			seenO = (c == 'O');
		}
		_delay_ms(1);
	}
	return 0;
}

void LED_Toggle_D13(void)
{
	PORTB ^= (1<<5); // PB5 -> D13
}

void LED_Blink3_Fail(void)
{
	for (int i=0;i<3;i++)
	{
		PORTB |= (1<<5);
		_delay_ms(120);
		PORTB &= ~(1<<5);
		_delay_ms(120);
	}
}

uint8_t JoinAP(void)
{
	char cmd[128];

	USART_FlushRx();
	USART_SendString("AT+CWMODE=1\r\n");
	if (!Wait_OK(2000))
	{
		return 0;
	}
	USART_FlushRx();

	// 큰따옴표 포함해서 만들기
	// 나중에 snprintf 안쓰는걸로 수정
	// AT+CWJAP="SSID","PASS"
	snprintf(cmd, sizeof(cmd),
	"AT+CWJAP=\"%s\",\"%s\"\r\n", SSID, PASS);

	USART_SendString(cmd);
	return Wait_OK(20000);
}

uint8_t Send_Intrusion(void)
{
	char cmd[128];

	USART_FlushRx();
	
	// 11800
	snprintf(cmd, sizeof(cmd), "AT+CIPSTART=\"TCP\",\"%s\",11800\r\n", LAPTOP_IP);
	USART_SendString(cmd);
	if (!Wait_OK(5000)) return 0;

	USART_FlushRx();
	// 9
	USART_SendString("AT+CIPSEND=9\r\n");
	_delay_ms(500);

	USART_SendString("INTRUSION");
	if (!Wait_OK(5000)) return 0;

	USART_SendString("AT+CIPCLOSE\r\n");
	return 1;
}

void start_wifi(void)
{
	DDRB |= (1<<5);
	PORTB &= ~(1<<5);
	USART_Init(DN_UBRR);
	_delay_ms(3000);
	
	USART_FlushRx();
	USART_SendString("ATE0\r\n");
	Wait_OK(300);

	uint8_t ok = JoinAP();
	
	if (!ok)
	{
		while(1){ LED_Blink3_Fail(); _delay_ms(500); }
	}
	
	for(int i=0;i<3;i++){ LED_Toggle_D13(); _delay_ms(700); }
	USART_FlushRx();
	USART_SendString("AT+CIFSR\r\n");
	
	if (Wait_OK(2000))
	{
		for(int i=0;i<2;i++){ LED_Toggle_D13(); _delay_ms(150); }
	} else
	{
		while(1){ LED_Blink3_Fail(); _delay_ms(500); }
	}
// 	while (1)
// 	{
// 		if (Send_Intrusion())
// 		{
// 			for (int i=0; i<2; i++)
// 			{
// 				LED_Toggle_D13();
// 				_delay_ms(100);
// 			}
// 		}
// 		else
// 		{
// 			LED_Blink3_Fail();
// 		}
// 
// 		_delay_ms(30000);
// 		
// 	}
}
