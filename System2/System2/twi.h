#ifndef TWI_H_
#define TWI_H_

#define F_CPU 16000000UL
#include <avr/io.h>

#define TWI_FREQ 100000UL
#define TWBR_VAL ((F_CPU / TWI_FREQ - 16) / 2)

void TWI_Init(void);
uint8_t TWI_Start(void);
uint8_t TWI_Write(uint8_t data);
void TWI_Stop(void);

#endif