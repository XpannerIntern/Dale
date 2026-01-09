#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h> // 인터럽트 헤더 추가
#include <stdint.h>
#include <string.h>
#include <stdio.h>         // sprintf 사용을 위해 추가
#include "Wifi_Module.h"

#define TWI_FREQ 100000UL
#define TWBR_VAL ((F_CPU / TWI_FREQ - 16) / 2)
#define LCD_ADDRESS (0x27 << 1)

#define E_BIT (1<<2)
#define RS_BIT (1<<0)
#define BL_BIT (1<<3)

// 핀 정의
//#define BUTTON_PIN PD2    // INT0
#define PIR_PIN    PD3    // PIR 센서 입력

// 전역 변수
volatile uint16_t intrusionCount = 0; // 침입 횟수 (인터럽트에서 사용하므로 volatile)
volatile uint8_t updateFlag = 0;      // LCD 갱신 플래그
volatile uint8_t isMonitoring = 0;
volatile uint8_t resetRequested = 0; // [추가] 리셋 메시지 출력 요청 깃발

void ADC_Init()
{
	ADMUX = (1 << REFS0);
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t ADC_Read()
{
	ADCSRA |= (1 << ADSC);
	while (ADCSRA & (1 << ADSC));
	return ADC;
}

// --- [기존 TWI 및 LCD 함수 시작] ---
void TWI_Init(void) {
	TWBR = (uint8_t)TWBR_VAL;
	TWSR = 0x00;
	TWCR = (1 << TWEN);
}

uint8_t TWI_Start(void) {
	TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
	while (!(TWCR & (1 << TWINT)));
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

void display_status(void) {
	char buffer[16];
	lcd_send_cmd(0x01); // 화면 클리어
	_delay_ms(2);
	
	lcd_set_cursor(0, 0);
	if (isMonitoring) {
		lcd_print("SYSTEM: ACTIVE");
	} else {
		lcd_print("SYSTEM: READY");
	}

	lcd_set_cursor(0, 1);
	sprintf(buffer, "Count: %u", intrusionCount);
	lcd_print(buffer);
}
// --- [기존 TWI 및 LCD 함수 끝] ---

// --- [ 인터럽트 1: PIR 센서 (PD3) ] ---
ISR(INT1_vect) {
	if (isMonitoring == 1){
		intrusionCount++;
		updateFlag = 1; // 화면 갱신 예약
		Send_Intrusion();
	}
}

// --- [ 타이머 1 인터럽트: 키패드 감시 (약 50ms 주기) ] ---
ISR(TIMER1_COMPA_vect) {
	uint16_t val = ADC_Read();
	static uint8_t buttonReleased = 1;

	if (val < 950) { // 버튼이 눌렸을 때
		if (buttonReleased) {
			// SW5 (약 741): 탐지 시작
			if (val > 650 && val < 850) {
				isMonitoring = 1;
				updateFlag = 1;
			}
			// SW4 (약 504): 횟수 초기화
			else if (val > 400 && val < 600) {
				isMonitoring = 0;
				intrusionCount = 0;
				resetRequested = 1; // [수정] 1초 메시지 출력을 위해 깃발 올림
			}
			buttonReleased = 0;
		}
		} else {
		buttonReleased = 1; // 손을 뗌
	}
}

void GPIO_Init(void) {
	// PIR 센서 핀(PD3) 입력 설정
	DDRD &= ~(1 << PIR_PIN);
	
	// 리셋 버튼 핀(PD2) 입력 설정 및 내부 풀업 저항 활성화
	//DDRD &= ~(1 << BUTTON_PIN);
	//PORTD |= (1 << BUTTON_PIN);
	
	// 외부 인터럽트 설정 (INT0)
	// ISC01=1, ISC00=0 : 하강 엣지(Falling Edge)에서 인터럽트 발생 (버튼 누를 때)
	EICRA |= (1 << ISC11);
	EICRA &= ~(1 << ISC10);
	EIMSK |= (1 << INT1); // INT0 활성화
	
	// 타이머 1 설정 (CTC 모드, 50ms 주기)
	TCCR1B |= (1 << WGM12) | (1 << CS12) | (1 << CS10); // CTC, 1024 분주
	OCR1A = 781; // 16MHz/1024/20Hz = 781.25 (약 50ms)
	TIMSK1 |= (1 << OCIE1A);
	
	ADC_Init();  // **중요: 누락되었던 부분**
	lcd_init();  // TWI_Init 포함
}

int main(void) {
	GPIO_Init();
	sei(); // 전역 인터럽트 활성화

	// 1. 시스템 시작 메시지
	lcd_set_cursor(0, 0);
	lcd_print("Intrusion Det.");
	lcd_set_cursor(0, 1);
	lcd_print("Starting...");
	_delay_ms(2000); // 2초간 대기
	
	start_wifi();
	
	display_status();

	while (1) {
		// [추가] SW4 리셋 메시지 처리
		if (resetRequested) {
			lcd_send_cmd(0x01); // 화면 지우기
			_delay_ms(2);
			lcd_set_cursor(0, 0);
			lcd_print("   RESET NOW!   "); // 리셋 메시지 출력
			lcd_set_cursor(0,1);
			lcd_print("Returning READY ");
		
			_delay_ms(1000); // 1초간 대기 (main 루프이므로 비교적 안전)
		
			resetRequested = 0; // 요청 처리 완료
			updateFlag = 1;     // 다시 원래 상태 화면으로 돌아가도록 설정
		}

		// 일반적인 화면 업데이트 처리
		if (updateFlag && !resetRequested) {
			display_status();
			updateFlag = 0;
		}
	}
		
	return 0;
}