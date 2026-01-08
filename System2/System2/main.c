#include "intruder_sys.h"
#include "twi.h"
#include "lcd_i2c.h"
#include <util/delay.h>

int main(void) {
	GPIO_Init();
	
	lcd_set_cursor(0, 0);
	lcd_print("Intrusion Det.");
	lcd_set_cursor(0, 1);
	lcd_print("Starting...");
	_delay_ms(2000);
	
	display_status();
	System_Process();

	while (1) {
/*		if (resetRequested) {
			lcd_send_cmd(0x01);
			_delay_ms(2);
			lcd_set_cursor(0, 0);
			lcd_print("    RESET NOW!   ");
			lcd_set_cursor(0,1);
			lcd_print("Returning READY ");
			_delay_ms(1000);
			
			resetRequested = 0;
			updateFlag = 1;
		}

		if (updateFlag && !resetRequested) {
			display_status();
			updateFlag = 0;
		}*/
	}
	return 0;
}