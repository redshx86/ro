// -------------------------------------------------------------------------------------------------
// RO controller MCU pinout

#pragma once

// -------------------------------------------------------------------------------------------------
// Discrete input

#define DI_PIN				PINB
#define DI_A_N				(1<<PB0)
#define DI_B_N				(1<<PB1)

#define DI_A_IS_ON()		!(DI_PIN & DI_A_N)
#define DI_B_IS_ON()		!(DI_PIN & DI_B_N)

// -------------------------------------------------------------------------------------------------
// Discrete output

#define DQ_PORT				PORTD
#define DQ_PIN				PIND
#define DQ_A_P				(1<<PD1)
#define DQ_B_P				(1<<PD2)
#define DQ_C_P				(1<<PD3)

#define DQ_A_ON()			DQ_PORT |= DQ_A_P
#define DQ_A_OFF()			DQ_PORT &= ~DQ_A_P
#define DQ_A_TGL()			DQ_PIN = DQ_A_P
#define DQ_A_IS_ON()		(DQ_PORT & DQ_A_P)

#define DQ_B_ON()			DQ_PORT |= DQ_B_P
#define DQ_B_OFF()			DQ_PORT &= ~DQ_B_P
#define DQ_B_TGL()			DQ_PIN = DQ_B_P
#define DQ_B_IS_ON()		(DQ_PORT & DQ_B_P)

#define DQ_C_ON()			DQ_PORT |= DQ_C_P
#define DQ_C_OFF()			DQ_PORT &= ~DQ_C_P
#define DQ_C_TGL()			DQ_PIN = DQ_C_P
#define DQ_C_IS_ON()		(DQ_PORT & DQ_C_P)

// -------------------------------------------------------------------------------------------------
// 7-segment display

#define SEG_3_0_PORT		PORTC	// PORTC[3:0]=SEG[3:0]
#define SEG_7_4_PORT		PORTD	// PORTD[7:4]=SEG[7:4]
#define SEG_3_0_P			0x0F
#define SEG_7_4_P			0xF0

#define SEL_PORT			PORTB
#define SEL_0_BIT			PB3
#define SEL_1_BIT			PB4
#define SEL_2_BIT			PB5
#define SEL_0_P				(1<<SEL_0_BIT)
#define SEL_1_P				(1<<SEL_1_BIT)
#define SEL_2_P				(1<<SEL_2_BIT)
#define SEL_ALL				(SEL_0_P|SEL_1_P|SEL_2_P)

#define SEG_WRITE(data) do {												\
		SEG_3_0_PORT = (SEG_3_0_PORT & ~SEG_3_0_P) | ((data) & SEG_3_0_P);	\
		SEG_7_4_PORT = (SEG_7_4_PORT & ~SEG_7_4_P) | ((data) & SEG_7_4_P);	\
	} while(0)

#define SEL_OFF()			SEL_PORT &= ~(SEL_0_P|SEL_1_P|SEL_2_P)
#define SEL_0_ON()			SEL_PORT |= SEL_0_P
#define SEL_1_ON()			SEL_PORT |= SEL_1_P
#define SEL_2_ON()			SEL_PORT |= SEL_2_P

// -------------------------------------------------------------------------------------------------
// Push button

#define BTN_PIN				PIND
#define BTN_PORT			PORTD
#define BTN_DDR				DDRD
#define BTN_N				(1<<PD0)

#define BTN_ENABLE() do {		\
		BTN_DDR &= ~BTN_N;		\
		BTN_PORT |= BTN_N;		\
	} while(0)
#define BTN_DISABLE() do {		\
		BTN_PORT &= ~BTN_N;		\
		BTN_DDR |= BTN_N;		\
	} while(0)
#define BTN_IS_PSH()		!(BTN_PIN & BTN_N)

// -------------------------------------------------------------------------------------------------
// Buzzer (3.2kHz)

#define BUZZ_FREQ			3200
#define BUZZ_P				(1<<PB2)	// OC1B

#define BUZZ_ON()			TCCR1A |= 1<<COM1B1
#define BUZZ_OFF()			TCCR1A &= ~(1<<COM1B1)
#define BUZZ_IS_ON()		(TCCR1A & (1<<COM1B1))

// Timer1:FastPWM(TOP=OCR1A) @ F_CPU
// OC1B -> Buzzer
#define BUZZ_ENABLE() do {							\
		PRR &= ~(1<<PRTIM1);						\
		OCR1A = F_CPU/BUZZ_FREQ-1;					\
		OCR1B = F_CPU/BUZZ_FREQ/2;					\
		TCCR1A = (1<<WGM11)|(1<<WGM10);				\
		TCCR1B = (1<<WGM13)|(1<<WGM12)|(1<<CS10);	\
	} while(0)
#define BUZZ_DISABLE() do {							\
		BUZZ_OFF();									\
		TCCR1B &= ~(1<<CS10);						\
		PRR |= 1<<PRTIM1;							\
	} while(0)

// -------------------------------------------------------------------------------------------------

// Timer0:Normal @ F_CPU/64
// OC0A -> Sys tick and Display
#define TICK_DISP_ENABLE() do {						\
		PRR &= ~(1<<PRTIM0);						\
		TCCR0B = (1<<CS01)|(1<<CS00);				\
		TIMSK0 = 1<<OCIE0A;							\
	} while(0)
#define TICK_DISP_DISABLE() do {					\
		TCCR0B = 0;									\
		PRR |= 1<<PRTIM0;							\
		SEG_WRITE(0);								\
		SEL_OFF();									\
	} while(0)

// -------------------------------------------------------------------------------------------------

// Timer2:Async @ F_32K/256
// OVF -> RTC tick
#define RTC_TICK_INIT() do {						\
		ASSR = 1<<AS2;								\
		TCNT2 = 0;									\
		OCR2A = 0;									\
		OCR2B = 0;									\
		TCCR2A = 0;									\
		TCCR2B = (1<<CS22)|(1<<CS21);				\
		while(ASSR & 0x1f)							\
			;										\
		TIMSK2 = 1<<TOIE2;							\
	} while(0)

// -------------------------------------------------------------------------------------------------
// ADC

#define ADMUX_AIN_C			((1<<REFS1)|(1<<REFS0)|6)	// 5.00V FS
#define ADMUX_VDC			((1<<REFS1)|(1<<REFS0)|7)	// 51.2V FS

// -------------------------------------------------------------------------------------------------

//#define PORTB_INIT		(0)
#define DDRB_INIT			(BUZZ_P|SEL_0_P|SEL_1_P|SEL_2_P)
//#define PORTC_INIT		(0)
#define DDRC_INIT			(SEG_3_0_P)
//#define PORTD_INIT		(0)
#define DDRD_INIT			(BTN_N|DQ_A_P|DQ_B_P|DQ_C_P|SEG_7_4_P)

// -------------------------------------------------------------------------------------------------
