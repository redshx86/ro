; --------------------------------------------------------------------------------------------------

#include <avr/io.h>
#include "macro.inc"
#include "adc.h"

; --------------------------------------------------------------------------------------------------

.global ADC_vect
.global adc_start
.global adc_buf

; --------------------------------------------------------------------------------------------------

ADC_vect:

	push	EL
	in		EL,SREG
	pushw	E,Z

	lds		EL,adc_cnt					;
	ldi		EH,0						; cnt<EL> = adc_cnt
	cpi		EL,ADC_NSAMP				; if(cnt<EL> < ADC_NSAMP)
	brsh	_adc_wend					; {
	ldiw	Z,adc_buf					;
	addw	Z,E,2						;     ptr<Z> = adc_buf + cnt<EL> * 2
	inc		EL							;
	sts		adc_cnt,EL					;     adc_cnt = ++cnt<EL>
	inw		E,ADCW						;
	stpw	Z,E							;     word[ptr<Z>++] = ADC
_adc_wend:								; }

	lds		EL,adc_cnt					; cnt<EL> = adc_cnt
	cpi		EL,ADC_NSAMP-1				; if(cnt<EL> >= ADC_NSAMP-1)
	brlo	_adc_end					; {
	brne	_adc_done					;     if(cnt<EL> == ADC_NSAMP-1) {
	in		EL,ADCSRA					;
	andi	EL,~((1<<ADIF)|(1<<ADATE))	;
	out		ADCSRA,EL					;         ADCSRA &= ~((1<<ADIF)|(1<<ADATE))
	rjmp	_adc_end					;     }
_adc_done:								;     else {
	sbi		GPIOR0,2					;         TASK_FLAG_SET(ADC_TASK_FLAG)
_adc_end:								; } }

	popw	Z,E
	out		SREG,EL
	pop		EL

	reti

; --------------------------------------------------------------------------------------------------

; void adc_start(uint8_t mux<EL>)

adc_start:
	sts		adc_cnt,CZERO				; adc_cnt = 0
	out		ADMUX,EL					; ADMUX = mux<EL>
	in		EL,ADCSRA					;
	ori		EL,(1<<ADSC)|(1<<ADATE)		;
	out		ADCSRA,EL					; ADCSRA |= (1<<ADSC)|(1<<ADATE)
	ret									;

; --------------------------------------------------------------------------------------------------

.section ".bss"

adc_buf:		.fill ADC_NSAMP, 2, 0
adc_cnt:		.byte 0

; --------------------------------------------------------------------------------------------------
