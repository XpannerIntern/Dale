#include "twi.h"

void TWI_Init(void) {
	TWBR = (uint8_t)TWBR_VAL;
	TWSR = 0x00;
	TWCR = (1 << TWEN);
}

uint8_t TWI_Start(void) {
	uint16_t timeout = 0;
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT))) {
		if (++timeout > 10000) { // 약 수십 ms 동안 응답 없으면 강제 종료
			return 1; // 에러 발생 리턴
		}
	}
	return (TWSR & 0xF8) != 0x08 && (TWSR & 0xF8) != 0x10;
}

uint8_t TWI_Write(uint8_t data) {
	TWDR = data;
	TWCR = (1 << TWINT) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
	return !((TWSR & 0xF8) == 0x18 || (TWSR & 0xF8) == 0x28);
}

void TWI_Stop(void) {
	TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN);
	while (TWCR & (1 << TWSTO));
}
