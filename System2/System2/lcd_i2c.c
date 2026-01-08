#include "lcd_i2c.h"
#include <util/delay.h>

void lcd_write_4bit(uint8_t data, uint8_t rs) {
	uint8_t temp = data | E_BIT | rs | BL_BIT;
	if (TWI_Start() == 0) {
		TWI_Write(LCD_ADDRESS);
		TWI_Write(temp);
		TWI_Stop();
	}
	_delay_us(1);
	temp &= ~E_BIT;
	if (TWI_Start() == 0) {
		TWI_Write(LCD_ADDRESS);
		TWI_Write(temp);
		TWI_Stop();
	}
	_delay_us(50);
}

void lcd_send_cmd(uint8_t cmd) {
	lcd_write_4bit(cmd & 0xF0, 0);
	lcd_write_4bit(cmd << 4, 0);
}

void lcd_send_data(uint8_t data) {
	lcd_write_4bit(data & 0xF0, RS_BIT);
	lcd_write_4bit(data << 4, RS_BIT);
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
	uint8_t address[] = {0x80, 0xC0};
	lcd_send_cmd(address[row] + col);
}

void lcd_init(void) {
	_delay_ms(50);
	TWI_Init();
	lcd_write_4bit(0x30, 0); _delay_ms(5);
	lcd_write_4bit(0x30, 0); _delay_us(150);
	lcd_write_4bit(0x30, 0);
	lcd_write_4bit(0x20, 0);
	lcd_send_cmd(0x28);
	lcd_send_cmd(0x0C);
	lcd_send_cmd(0x06);
	lcd_send_cmd(0x01);
	_delay_ms(2);
}

void lcd_print(const char *str) {
	while (*str) lcd_send_data(*str++);
}

