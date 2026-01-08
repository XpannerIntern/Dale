#ifndef LCD_I2C_H_
#define LCD_I2C_H_

#include "twi.h"

#define TWI_FREQ 100000UL
#define LCD_ADDRESS (0x27 << 1)

#define E_BIT (1<<2)
#define RS_BIT (1<<0)
#define BL_BIT (1<<3)

void lcd_init(void);
void lcd_write_4bit(uint8_t data, uint8_t rs);
void lcd_send_cmd(uint8_t cmd);
void lcd_send_data(uint8_t data);
void lcd_print(const char *str);
void lcd_set_cursor(uint8_t col, uint8_t row);

#endif

