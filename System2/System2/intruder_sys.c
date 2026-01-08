#include "intruder_sys.h"
#include <stdio.h>
#include <util/delay.h>

// 변수 정의
volatile uint16_t intrusionCount = 0;
volatile uint8_t updateFlag = 0;
volatile uint8_t isMonitoring = 0;
volatile uint8_t resetRequested = 0;
static volatile uint8_t blinkTimer = 0;
static volatile uint8_t blinkCount = 0;

void ADC_Init() {
	ADMUX = (1 << REFS0);
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t ADC_Read() {
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
	return ADC;
}

void display_status(void) {
	char buffer[16];
	lcd_send_cmd(0x01);
	_delay_ms(2);
	
	lcd_set_cursor(0, 0);
	if (isMonitoring) lcd_print("SYSTEM: ACTIVE");
	else lcd_print("SYSTEM: READY");

	lcd_set_cursor(0, 1);
	sprintf(buffer, "Count: %u", intrusionCount);
	lcd_print(buffer);
}

// ISR 코드는 이 파일에 위치 (하드웨어 제어 로직이므로)
ISR(INT1_vect) {
	if (isMonitoring == 1){
		intrusionCount++;
		updateFlag = 1;
	}
}

ISR(TIMER1_COMPA_vect) {
	uint16_t val = ADC_Read();
	static uint8_t buttonReleased = 1;

	if (val < 950) {
		if (buttonReleased) {
			if (val > 650 && val < 850) { // SW5
				isMonitoring = 1;
				updateFlag = 1;
				PORTB |= (1 << PORTB1);
			}
			else if (val > 400 && val < 600) { // SW4
				isMonitoring = 0;
				intrusionCount = 0;
				resetRequested = 1;
				blinkCount = 10;
				blinkTimer = 0;
			}
			buttonReleased = 0;
		}
		} else {
		buttonReleased = 1;
	}

	if (blinkCount > 0) {
		blinkTimer++;
		if (blinkTimer > 5) {
			PORTB ^= (1 << PORTB0);
			blinkCount--;
			blinkTimer = 0;
			if (blinkCount == 0) PORTB &= ~(1 << PORTB0);
		}
	}
}

void GPIO_Init(void) {
	DDRD &= ~(1 << PIR_PIN);
	DDRB |= (1 << DDB0) | (1 << DDB1);
	
	EICRA |= (1 << ISC11);
	EIMSK |= (1 << INT1);
	
	TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10);
	OCR1A = 781;
	TIMSK1 |= (1 << OCIE1A);
	
	ADC_Init();
	sei();
	lcd_init();
	
}

void System_Process(void)
{
	while (1) {
		if (resetRequested) {
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
		}
	}
	
}