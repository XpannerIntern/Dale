#ifndef INTRUDER_SYS_H_
#define INTRUDER_SYS_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include "lcd_i2c.h"

// 핀 정의
#define PIR_PIN PD3

// 전역 변수 공유 (extern 키워드 사용)
extern volatile uint16_t intrusionCount;
extern volatile uint8_t updateFlag;
extern volatile uint8_t isMonitoring;
extern volatile uint8_t resetRequested;

void GPIO_Init(void);
void ADC_Init(void);
uint16_t ADC_Read(void);
void display_status(void);
void System_Process(void);

#endif