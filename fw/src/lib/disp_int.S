; --------------------------------------------------------------------------------------------------

#include <avr/io.h>
#include "../hwconf.h"
#include "disp.h"
#include "macro.inc"

; --------------------------------------------------------------------------------------------------

.global TIMER0_COMPA_vect
.global disp_buf
.global sseg_digit

.extern t_tick_src

; --------------------------------------------------------------------------------------------------

// 1.6ms int
TIMER0_COMPA_vect:

	push	EL							;
	in		EL,SREG						;
	pushw	E,Z							;

	; Display refresh
	in		EL,SEL_PORT					;
	andi	EL,~SEL_ALL					;
	out		SEL_PORT,EL					; SEL_PORT &= ~SEL_ALL
	lds		EL,disp_cyc					; cyc<EL> = disp_cyc
	ldi		EH,0						;
	ldiw	Z,disp_buf					;
	addw	Z,E							; data_ptr<Z> = disp_buf + cyc<EL
	mov		EH,EL						; cyc<EH> = cyc<EL>
	inc		EL							; cyc_next<EL> = cyc<EL> + 1
	cpi		EL,DISP_N					; if(cyc_next<EL> == DISP_N)
	brlo	_disp_nwrap					; {
	ldi		EL,0						;     cyc_next<EL> = 0
_disp_nwrap:							; }
	sts		disp_cyc,EL					; disp_cyc = cyc_next<EL>
	ld		ZL,Z						; data<ZL> = byte[data_ptr<Z>]
	mov		ZH,ZL						;
	andi	ZL,SEG_3_0_P				; seg_3_0<ZL> = data<ZL> & SEG_3_0_P
	andi	ZH,SEG_7_4_P				; seg_7_4<ZH> = data<ZH> & SEG_7_4_P
	in		EL,SEG_3_0_PORT				;
	andi	EL,~SEG_3_0_P				;
	or		EL,ZL						;
	out		SEG_3_0_PORT,EL				; SEG_3_0_PORT = (SEG_3_0_PORT & ~SEG_3_0_P) | seg_3_0<ZL>
	in		EL,SEG_7_4_PORT				;
	andi	EL,~SEG_7_4_P				;
	or		EL,ZH						;
	out		SEG_7_4_PORT,EL				; SEG_7_4_PORT = (SEG_7_4_PORT & ~SEG_7_4_P) | seg_7_4<ZH>
	cpi		EH,1						;
	breq	_disp_sel_1					;
	cpi		EH,2						;
	breq	_disp_sel_2					; if(disp_cyc == 0)
	sbi		SEL_PORT,SEL_0_BIT			;     SEL_PORT |= SEL_0_P
	rjmp	_disp_sel_end				; }
_disp_sel_1:							; else if(disp_cyc == 1) {
	sbi		SEL_PORT,SEL_1_BIT			;     SEL_PORT |= SEL_1_P
	rjmp	_disp_sel_end				; }
_disp_sel_2:							; else if(disp_cyc == 2) {
	sbi		SEL_PORT,SEL_2_BIT			;     SEL_PORT |= SEL_2_P
	ldsw	E,t_tick_src				;
	addiw	E,1							;
	stsw	t_tick_src,E				;     t_tick_src++
	sbi		GPIOR0,0					;     TASK_FLAG_SET(OS_TICK_UPD_FLAG)
_disp_sel_end:							; }

	in		EL,OCR0A					;
	subi	EL,-100						;
	out		OCR0A,EL					; OCR0A += 100

	popw	Z,E							;
	out		SREG,EL						;
	pop		EL							;

	reti								;

; --------------------------------------------------------------------------------------------------

.section ".bss"

disp_cyc:	.byte 0
disp_buf:	.fill DISP_N, 1, 0

; --------------------------------------------------------------------------------------------------

.section ".progmem.data"

sseg_digit:	.byte	SSEG_0, SSEG_1, SSEG_2, SSEG_3, SSEG_4, SSEG_5, SSEG_6, SSEG_7, SSEG_8, SSEG_9

; --------------------------------------------------------------------------------------------------
