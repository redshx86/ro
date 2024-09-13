; --------------------------------------------------------------------------------------------------
; macro library for word and dword operations
; --------------------------------------------------------------------------------------------------

; words (S:call-saved, L:low, H:high, Z:zero, T:temp)
;  Z= ZH, ZL | H   |	  |
;  Y= YH, YL | HS  |	  |
;  X= XH, XL | H   |	  |
;  E= EH, EL | H   | ARG0 | RET64 RET32 RET16 EL=RET8
;  D= DH, DL | H   | ARG1 | RET64 RET32
;  C= CH, CL | H   | ARG2 | RET64
;  B= BH, BL | H   | ARG3 | RET64
;  A= AH, AL | HS  | ARG4 |
; T7=T7H,T7L | LS  | ARG5 |
; T6=T6H,T6L | LS  | ARG6 |
; T5=T5H,T5L | LS  | ARG7 |
; T4=T4H,T4L | LS  | ARG8 |
; T3=T3H,T3L | LS  |	  |
; T2=T2H,T2L | LS  |	  |
; T1=T1H,T1L | LS  |	  |
; T0=T0H,T0L | LZT |	  | CZEROW=T0H,T0H CZERO=T0H CTEMP=T0L

; dwords: ZX ZY YX XE ED DC CB BA T76 T65 T54 T43 T32 T21 T10

; In, Out:
;	cbi		addr, n[, temp]			| emits cbi, in+andi+out or lds+andi+sts. SRAM address
;	sbi		addr, n[, temp]			| emits sbi, in+ori+out or lds+ori+sts. SRAM address
;	sbic	addr, n[, temp]			| emits sbic, in+sbrc or lds+sbrc. SRAM address
;	sbis	addr, n[, temp]			| emits sbis, in+sbrs or lds+sbrs. SRAM address
;	in		byte, addr				| emits in or lds, SRAM address
;	out		byte, addr				| emits out or sts, SRAM address
;	outi	addr, val, temp			| emits ldi+out or ldi+sts. SRAM address. can be used as stsi
;	inw		word, addr				| in word. low -> high
;	outw	addr, word				| out word. high -> low
;	outiw	addr, val, temp			| outi word. high -> low

; Move:
;	movw	word, word				| emits movw or mov+mov
;	mov32	dword, dword			| 2* movw macro

; Arithmetic:
;	addw	word, word[, repeat]	| emits add+adc
;	subw	word, word[, repeat]	| emits sub+sbc
;	cpw		word, word				| emits cp+cpc
;	negw	word[, zero]			| emits neg+adc+neg
;	add32	dword, dword[, repeat]	| emits add+adc
;	sub32	dword, dword[, repeat]	| emits sub+sbc
;	cp32	dword, dword			| emits cp+cpc

; Arithmetic (immediate):
;	addiw	word, imm16				| FLAG-UNSAFE if __b0(imm16) == 0. emits adiw or subi+sbci
;	subiw	word, imm16				| FLAG-UNSAFE if __b0(imm16) == 0. emits sbiw or subi+sbci
;	addiwf	word, imm16				| emits adiw or subi+sbci
;	subiwf	word, imm16				| emits sbiw or subi+sbci
;	cpiw	word, imm16[, temp]		| emits subi+sbci, ldi+cpi+cpc or ldi+cp+cpc
;	cpiwb	word, imm8[, zero]		| emits cpi+cpc
;	cpiws	word, addr				| emits ldi+cpi+cpc (SRAM address compatible)
;	addi32	dword, imm32			| FLAG-UNSAFE if __b0(imm32) == 0. emits subi+sbci
;	subi32	dword, imm32			| FLAG-UNSAFE if __b0(imm32) == 0. emits subi+sbci
;	addi32f	dword, imm32			| emits subi+sbci
;	subi32f	dword, imm32			| emits subi+sbci
;	cpi32	dword, imm32[, temp]	| emits subi+sbci or ldi+cpi+cpc
;	cpi32b	dword, imm8[, zero]		| emits cpi+cpc
;	cpi32w	dword,imm16,temp[,zero]	| emits ldi+cpi+cpc

; Logic:
;	andw	word, word				| emits 2*and
;	orw		word, word				| emits 2*or
;	eorw	word, word				| emits 2*eor
;	comw	word, word				| emits 2*com
;	and32	dword, dword			| emits 4*and
;	or32	dword, dword			| emits 4*or
;	eor32	dword, dword			| emits 4*eor
;	com32	dword, dword			| emits 4*com

; Logic (immediate):
;	andiw	word, imm16				| emits 2*andi (0xff bytes ignored)
;	oriw	word, imm16				| emits 2*ori (0x00 bytes ignored)
;	andi32	dword, imm32			| emits 4*andi (0xff bytes ignored)
;	ori32	dword, imm32			| emits 4*ori (0x00 bytes ignored)

; Shift:
;	lsl		byte[, repeat]			| emits lsl (optional repeat)
;	lsr		byte[, repeat]			| emits lsr (optional repeat)
;	asr		byte[, repeat]			| emits asr (optional repeat)
;	rol		byte[, repeat]			| emits rol (optional repeat)
;	ror		byte[, repeat]			| emits ror (optional repeat)
;	lslw	word[, repeat]			| FLAG-UNSAFE if repeat == 8. emits mov+eor, then lsl+rol
;	lsrw	word[, repeat]			| FLAG-UNSAFE if repeat == 8. emits mov+eor, then lsr+ror
;	asrw	word[, repeat]			| FLAG-UNSAFE if repeat == 8. emits mov+eor, then asr+ror
;	lslwf	word[, repeat]			| emits lsl+rol
;	lsrwf	word[, repeat]			| emits lsr+ror
;	asrwf	word[, repeat]			| emits asr+ror
;	rolw	word[, repeat]			| emits 2*rol
;	rorw	word[, repeat]			| emits 2*ror
;	rotlw	word, repeat, temp		| rotate word left
;	rotrw	word, repeat, temp		| rotate word right
;	lsl32	dword[, repeat]			| FLAG-UNSAFE if repeat == 8,16,24. emits mov+eor, then lsl+rol
;	lsr32	dword[, repeat]			| FLAG-UNSAFE if repeat == 8,16,24. emits mov+eor, then lsr+ror
;	asr32	dword[, repeat]			| FLAG-UNSAFE if repeat == 8,16,24. emits mov+eor, then asr+ror
;	lsl32f	dword[, repeat]			| emits lsl+rol
;	lsr32f	dword[, repeat]			| emits lsr+ror
;	asr32f	dword[, repeat]			| emits asr+ror
;	rol32	dword[, repeat]			| emits 4*rol
;	ror32	dword[, repeat]			| emits 4*ror
;	rotl32	dword, repeat, temp		| rotate dword left
;	rotr32	dword, repeat, temp		| rotate dword right

; Load, store (normal byte order, low -> high):
;	ld		byte, ptr				| emits ld reg,ptr. NOTE: ptr+ or -ptr is not supported
;	ldp		byte, ptr				| emits ld reg,ptr+
;	mld		byte, ptr				| emits ld reg,-ptr
;	ldd		byte, ptr, disp			| emits ldd reg,ptr+disp
;	lpm		byte, Z					| emits lpm reg,Z. NOTE: ptr+ or -ptr is not supported
;	lpmp	byte, Z					| emits lpm reg,Z+
;	st		ptr, byte				| emits st ptr,reg. NOTE: ptr+ or -ptr is not supported
;	stp		ptr, byte				| emits st ptr+,reg
;	mst		ptr, byte				| emits st -ptr,reg
;	std		ptr, disp, byte			| emits std ptr+disp,reg
;	ldiw	word, imm16/addr		| emits 2*ldi (SRAM address compatible)
;	ldsw	word, addr				| emits 2*lds reg,addr+i
;	ldpw	word, ptr				| emits 2*ld reg,ptr+
;	mldw	word, ptr				| emits 2*ld reg,-ptr
;	lddw	word, ptr, disp			| emits 2*ldd reg,ptr+disp+i
;	lpmpw	word, Z					| emits 2*lpm reg,Z
;	stsw	addr, word				| emits 2*sts addr+i,reg
;	stpw	ptr, word				| emits 2*st ptr+,reg
;	mstw	ptr, word				| emits 2*st -ptr,reg
;	stdw	ptr, disp, word			| emits 2*std ptr+disp+i,reg
;	ldi32	dword, imm32			| emits 4*ldi
;	lds32	dword, addr				| emits 4*lds reg,addr+i
;	ldp32	dword, ptr				| emits 4*ld reg,ptr+
;	mld32	dword, ptr				| emits 4*ld reg,-ptr
;	ldd32	dword, ptr, disp		| emits 4*ldd reg,ptr+disp+i
;	lpmp32	dword, Z				| emits 4*lpm reg,Z
;	sts32	addr, dword				| emits 4*sts addr+i,reg
;	stp32	ptr, dword				| emits 4*st ptr+,reg
;	mst32	ptr, dword				| emits 4*st -ptr,reg
;	std32	ptr, disp, dword		| emits 4*std ptr+disp+i,reg

; Load, store (reversed byte order, high -> low):
;	ldswr	word, addr				| emits 2*lds reg,addr+i
;	ldpwr	word, ptr				| emits 2*ld reg,ptr+
;	mldwr	word, ptr				| emits 2*ld reg,-ptr
;	lddwr	word, ptr, disp			| emits 2*ldd reg,ptr+disp+i
;	lpmpwr	word, Z					| emits 2*lpm reg,Z
;	stswr	addr, word				| emits 2*sts addr+i,reg
;	stpwr	ptr, word				| emits 2*st ptr+,reg
;	mstwr	ptr, word				| emits 2*st -ptr,reg
;	stdwr	ptr, disp, word			| emits 2*std ptr+disp+i,reg
;	lds32r	dword, addr				| emits 4*lds reg,addr+i
;	ldp32r	dword, ptr				| emits 4*ld reg,ptr+
;	mld32r	dword, ptr				| emits 4*ld reg,-ptr
;	ldd32r	dword, ptr, disp		| emits 4*ldd reg,ptr+disp+i
;	lpmp32r	dword, Z				| emits 4*lpm reg,Z
;	sts32r	addr, dword				| emits 4*sts addr+i,reg
;	stp32r	ptr, dword				| emits 4*st ptr+,reg
;	mst32r	ptr, dword				| emits 4*st -ptr,reg
;	std32r	ptr, disp, dword		| emits 4*std ptr+disp+i,reg

; Push/pop:
;	push	reg[, ...]				| push bytes(s)
;	pop		reg[, ...]				| pop bytes(s)
;	pushw	word[, ...]				| push word(s)
;	popw	word[, ...]				| pop word(s)
;	push32	dword[, ...]			| push dword(s)
;	pop32	dword[, ...]			| pop dword(s)

; Multiply:
;	mulw	dword,word,word, zero	; DESTROYS T0H. u16 = u8 * u8
;	mulsw	dword,word,word, zero	; DESTROYS T0H. s16 = s8 * s8
;	fmulw	dword,word,word, zero	; DESTROYS T0H. u16 = (u8 * u8) << 1
;	fmulsw	dword,word,word, zero	; DESTROYS T0H. s16 = (s8 * s8) << 1

; NOP:
;	nop		[count]					| emits rjmp and nop instructions

; --------------------------------------------------------------------------------------------------
; Helper macros
; Need to compare with both uppercase and lowercase registers in the gnu assembler!
; --------------------------------------------------------------------------------------------------

#define __IS_REG_PAIR(rh,rl)											\
  (																		\
	( ((rl ==  R0) || (rl ==  r0)) && ((rh ==  R1) || (rh ==  r1)) ) || \
	( ((rl ==  R2) || (rl ==  r2)) && ((rh ==  R3) || (rh ==  r3)) ) || \
	( ((rl ==  R4) || (rl ==  r4)) && ((rh ==  R5) || (rh ==  r5)) ) || \
	( ((rl ==  R6) || (rl ==  r6)) && ((rh ==  R7) || (rh ==  r7)) ) || \
	( ((rl ==  R8) || (rl ==  r8)) && ((rh ==  R9) || (rh ==  r9)) ) || \
	( ((rl == R10) || (rl == r10)) && ((rh == R11) || (rh == r11)) ) || \
	( ((rl == R12) || (rl == r12)) && ((rh == R13) || (rh == r13)) ) || \
	( ((rl == R14) || (rl == r14)) && ((rh == R15) || (rh == r15)) ) || \
	( ((rl == R16) || (rl == r16)) && ((rh == R17) || (rh == r17)) ) || \
	( ((rl == R18) || (rl == r18)) && ((rh == R19) || (rh == r19)) ) || \
	( ((rl == R20) || (rl == r20)) && ((rh == R21) || (rh == r21)) ) || \
	( ((rl == R22) || (rl == r22)) && ((rh == R23) || (rh == r23)) ) || \
	( ((rl == R24) || (rl == r24)) && ((rh == R25) || (rh == r25)) ) || \
	( ((rl == R26) || (rl == r26)) && ((rh == R27) || (rh == r27)) ) || \
	( ((rl == R28) || (rl == r28)) && ((rh == R29) || (rh == r29)) ) || \
	( ((rl == R30) || (rl == r30)) && ((rh == R31) || (rh == r31)) )	\
  )

#define __IS_REG_PAIR_ADIW(rh,rl)										\
  (																		\
	( ((rl == R24) || (rl == r24)) && ((rh == R25) || (rh == r25)) ) || \
	( ((rl == R26) || (rl == r26)) && ((rh == R27) || (rh == r27)) ) || \
	( ((rl == R28) || (rl == r28)) && ((rh == R29) || (rh == r29)) ) || \
	( ((rl == R30) || (rl == r30)) && ((rh == R31) || (rh == r31)) )	\
  )

#define __IS_LOW_REG(rr)												\
  (																		\
	(rr ==	R0) || (rr ==  R1) || (rr ==  R2) || (rr ==	 R3) ||			\
	(rr ==	R4) || (rr ==  R5) || (rr ==  R6) || (rr ==	 R7) ||			\
	(rr ==	R8) || (rr ==  R9) || (rr == R10) || (rr == R11) ||			\
	(rr == R12) || (rr == R13) || (rr == R14) || (rr == R15) ||			\
	(rr ==	r0) || (rr ==  r1) || (rr ==  r2) || (rr ==	 r3) ||			\
	(rr ==	r4) || (rr ==  r5) || (rr ==  r6) || (rr ==	 r7) ||			\
	(rr ==	r8) || (rr ==  r9) || (rr == r10) || (rr == r11) ||			\
	(rr == r12) || (rr == r13) || (rr == r14) || (rr == r15)			\
  )

#define __IS_HIGH_REG(rr)												\
  (																		\
	(rr == R16) || (rr == R17) || (rr == R18) || (rr == R19) ||			\
	(rr == R20) || (rr == R21) || (rr == R22) || (rr == R23) ||			\
	(rr == R24) || (rr == R25) || (rr == R26) || (rr == R27) ||			\
	(rr == R28) || (rr == R29) || (rr == R30) || (rr == R31) ||			\
	(rr == r16) || (rr == r17) || (rr == r18) || (rr == r19) ||			\
	(rr == r20) || (rr == r21) || (rr == r22) || (rr == r23) ||			\
	(rr == r24) || (rr == r25) || (rr == r26) || (rr == r27) ||			\
	(rr == r28) || (rr == r29) || (rr == r30) || (rr == r31)			\
  )

#define __IS_Z_PTR(rh,rl)	(((rl == R30) || (rl == r30)) && ((rh == R31) || (rh == r31)))
#define __IS_Y_PTR(rh,rl)	(((rl == R28) || (rl == r28)) && ((rh == R29) || (rh == r29)))
#define __IS_X_PTR(rh,rl)	(((rl == R26) || (rl == r26)) && ((rh == R27) || (rh == r27)))

#define __BIT_ACCESS(addr)	(((addr) >= 0x20) && ((addr) < 0x40))
#define __SFR_ACCESS(addr)	(((addr) >= 0x20) && ((addr) < 0x60))
#define __SFR_INDEX(addr)	((addr) - 0x20)

#define __b0(val)			( (val)        & 0xff)
#define __b1(val)			(((val) >>  8) & 0xff)
#define __b2(val)			(((val) >> 16) & 0xff)
#define __b3(val)			(((val) >> 24) & 0xff)
#define __w0(val)			( (val)        & 0xffff)
#define __w1(val)			(((val) >> 16) & 0xffff)

#define __b0n(val)			( (-(val))        & 0xff)
#define __b1n(val)			(((-(val)) >>  8) & 0xff)
#define __b2n(val)			(((-(val)) >> 16) & 0xff)
#define __b3n(val)			(((-(val)) >> 24) & 0xff)
#define __w0n(val)			( (-(val))        & 0xffff)
#define __w1n(val)			(((-(val)) >> 16) & 0xffff)

; --------------------------------------------------------------------------------------------------
; In, out
; --------------------------------------------------------------------------------------------------

; BIT

.macro __cbi addr, n, temp=TEMP_REG
  .if __BIT_ACCESS(\addr)
	cbi		__SFR_INDEX(\addr),\n
  .elseif __SFR_ACCESS(\addr)
	in		\temp,__SFR_INDEX(\addr)
	andi	\temp,~(1<<\n)
	out		__SFR_INDEX(\addr),\temp
  .else
	lds		\temp,\addr
	andi	\temp,~(1<<\n)
	sts		\addr,\temp
  .endif
.endm

.macro __sbi addr, n, temp=TEMP_REG
  .if __BIT_ACCESS(\addr)
	sbi		__SFR_INDEX(\addr),\n
  .elseif __SFR_ACCESS(\addr)
	in		\temp,__SFR_INDEX(\addr)
	ori		\temp,1<<\n
	out		__SFR_INDEX(\addr),\temp
  .else
	lds		\temp,\addr
	ori		\temp,1<<\n
	sts		\addr,\temp
  .endif
.endm

.macro __sbic addr, n, temp=R0
  .if __BIT_ACCESS(\addr)
	sbic	__SFR_INDEX(\addr),\n
  .elseif __SFR_ACCESS(\addr)
	in		\temp,__SFR_INDEX(\addr)
	sbrc	\temp,\n
  .else
	lds		\temp,\addr
	sbrc	\temp,\n
  .endif
.endm

.macro __sbis addr, n, temp=R0
  .if __BIT_ACCESS(\addr)
	sbis	__SFR_INDEX(\addr),\n
  .elseif __SFR_ACCESS(\addr)
	in		\temp,__SFR_INDEX(\addr)
	sbrs	\temp,\n
  .else
	lds		\temp,\addr
	sbrs	\temp,\n
  .endif
.endm

; BYTE

.macro __in rd,addr
  .if __SFR_ACCESS(\addr)
	in		\rd,__SFR_INDEX(\addr)
  .else
	lds		\rd,\addr
  .endif
.endm

.macro __out addr,rs
  .if __SFR_ACCESS(\addr)
	out		__SFR_INDEX(\addr),\rs
  .else
	sts		\addr,\rs
  .endif
.endm

.macro outi addr,val, temp=TEMP_REG
	ldi		\temp,\val
	__out	\addr,\temp
.endm

.macro inw rh,rl, addr
	__in	\rl,\addr+0
	__in	\rh,\addr+1
.endm

; WORD

.macro outw addr, rh,rl
	__out	\addr+1,\rh
	__out	\addr+0,\rl
.endm

.macro outiw addr,val, temp=TEMP_REG
	ldi		\temp,__b1(\val)
	__out	\addr+1,\temp
  .if __b0(\val) != __b1(\val)
	ldi		\temp,__b0(\val)
  .endif
	__out	\addr+0,\temp
.endm

; --------------------------------------------------------------------------------------------------
; Move
; --------------------------------------------------------------------------------------------------

; WORD

.macro __movw th,tl, rh,rl
  .if __IS_REG_PAIR(\th,\tl) && __IS_REG_PAIR(\rh,\rl)
	movw	\tl,\rl
  .else
	mov		\tl,\rl
	mov		\th,\rh
  .endif
.endm

; DWORD

.macro mov32 td,tc,tb,ta, rd,rc,rb,ra
	__movw	\tb,\ta, \rb,\ra
	__movw	\td,\tc, \rd,\rc
.endm

; --------------------------------------------------------------------------------------------------
; Arithmetic
; --------------------------------------------------------------------------------------------------

; WORD

.macro	addw th,tl, rh,rl, n=1
  .rept \n
	add		\tl,\rl
	adc		\th,\rh
  .endr
.endm

.macro	subw th,tl, rh,rl, n=1
  .rept \n
	sub		\tl,\rl
	sbc		\th,\rh
  .endr
.endm

.macro	cpw th,tl, rh,rl
	cp		\tl,\rl
	cpc		\th,\rh
.endm

.macro	negw rh,rl, zero=R1
	neg		\rl
	adc		\rh,\zero
	neg		\rh
.endm

; DWORD

.macro	add32 td,tc,tb,ta, rd,rc,rb,ra, n=1
  .rept \n
	add		\ta,\ra
	adc		\tb,\rb
	adc		\tc,\rc
	adc		\td,\rd
  .endr
.endm

.macro	sub32 td,tc,tb,ta, rd,rc,rb,ra, n=1
  .rept \n
	sub		\ta,\ra
	sbc		\tb,\rb
	sbc		\tc,\rc
	sbc		\td,\rd
  .endr
.endm

.macro	cp32 td,tc,tb,ta, rd,rc,rb,ra
	cp		\ta,\ra
	cpc		\tb,\rb
	cpc		\tc,\rc
	cpc		\td,\rd
.endm

; --------------------------------------------------------------------------------------------------
; Arithmetic (immediate)
; --------------------------------------------------------------------------------------------------

; WORD

.macro	addiw rh,rl, val
 .if __w0(\val) != 0
  .if __b0(\val) == 0
	subi	\rh,__b1n(\val)
  .elseif __IS_REG_PAIR_ADIW(\rh,\rl) && (__w0(\val) < 64)
	adiw	\rl,__w0(\val)
  .elseif __IS_REG_PAIR_ADIW(\rh,\rl) && (__w0n(\val) < 64)
	sbiw	\rl,__w0n(\val)
  .else
	subi	\rl,__b0n(\val)
	sbci	\rh,__b1n(\val)
  .endif
 .endif
.endm

.macro	subiw rh,rl, val
 .if __w0(\val) != 0
  .if __b0(\val) == 0
	subi	\rh,__b1(\val)
  .elseif __IS_REG_PAIR_ADIW(\rh,\rl) && (__w0(\val) < 64)
	sbiw	\rl,__w0(\val)
  .elseif __IS_REG_PAIR_ADIW(\rh,\rl) && (__w0n(\val) < 64)
	adiw	\rl,__w0n(\val)
  .else
	subi	\rl,__b0(\val)
	sbci	\rh,__b1(\val)
  .endif
 .endif
.endm

.macro	addiwf rh,rl, val
  .if __IS_REG_PAIR_ADIW(\rh,\rl) && (__w0(\val) < 64)
	adiw	\rl,__w0(\val)
  .else
	subi	\rl,__b0n(\val)
	sbci	\rh,__b1n(\val)
  .endif
.endm

.macro	subiwf rh,rl, val
  .if __IS_REG_PAIR_ADIW(\rh,\rl) && (__w0(\val) < 64)
	sbiw	\rl,__w0(\val)
  .else
	subi	\rl,__b0(\val)
	sbci	\rh,__b1(\val)
  .endif
.endm

.macro	cpiw rh,rl, val, temp=TEMP_REG
 .if __w0(\val) == 0
  .if __IS_REG_PAIR_ADIW(\rh,\rl)
	sbiw	\rl,0
  .else
	subi	\rl,0
	sbci	\rh,0
  .endif
 .elseif __b1(\val) == __b0(\val)
	ldi		\temp,__b0(\val)
	cp		\rl,\temp
	cpc		\rh,\temp
 .elseif __IS_HIGH_REG(\rl)
	cpi		\rl,__b0(\val)
	ldi		\temp,__b1(\val)
	cpc		\rh,\temp
 .else
	ldi		\temp,__b0(\val)
	cp		\rl,\temp
	ldi		\temp,__b1(\val)
	cpc		\rh,\temp
 .endif
.endm

.macro cpiwb rh,rl, val, zero=R1
 .if (__w0(\val) == 0) && __IS_REG_PAIR_ADIW(\rh,\rl)
	sbiw	\rl,0
 .else
  .if __b0(\val) == 0
	cp		\rl,\zero
  .else
	cpi		\rl,\val
  .endif
	cpc		\rh,\zero
 .endif
.endm

.macro cpiws rh,rl, addr, temp=TEMP_REG
	cpi		\rl,lo8(\addr)
	ldi		\temp,hi8(\addr)
	cpc		\rh,\temp
.endm

; DWORD

.macro	addi32 rd,rc,rb,ra, val
 .if __b0(\val) != 0
  .if __IS_REG_PAIR_ADIW(\rd,\rc) && (__w0n(\val) < 64)
	sbiw	\ra,__w0n(\val)
  .else
	subi	\ra,__b0n(\val)
	sbci	\rb,__b1n(\val)
  .endif
	sbci	\rc,__b2n(\val)
	sbci	\rd,__b3n(\val)
 .elseif __b1(\val) != 0
	subi	\rb,__b1n(\val)
	sbci	\rc,__b2n(\val)
	sbci	\rd,__b3n(\val)
 .elseif __b2(\val) != 0
  .if __IS_REG_PAIR_ADIW(\rd,\rc) && (__w1(\val) < 64)
	adiw	\rc,__w1(\val)
  .elseif __IS_REG_PAIR_ADIW(\rd,\rc) && (__w1n(\val) < 64)
	sbiw	\rc,__w1n(\val)
  .else
	subi	\rc,__b2n(\val)
	sbci	\rd,__b3n(\val)
  .endif
 .elseif __b3(\val) != 0
	subi	\rd,__b3n(\val)
 .endif
.endm

.macro	subi32 rd,rc,rb,ra, val
 .if __b0(\val) != 0
  .if __IS_REG_PAIR_ADIW(\rb,\ra) && (__w0(\val) < 64)
	sbiw	\ra,__w0(\val)
  .else
	subi	\ra,__b0(\val)
	sbci	\rb,__b1(\val)
  .endif
	sbci	\rc,__b2(\val)
	sbci	\rd,__b3(\val)
 .elseif __b1(\val) != 0
	subi	\rb,__b1(\val)
	sbci	\rc,__b2(\val)
	sbci	\rd,__b3(\val)
 .elseif __b2(\val) != 0
  .if __IS_REG_PAIR_ADIW(\rd,\rc) && (__w1(\val) < 64)
	sbiw	\rc,__w1(\val)
  .elseif __IS_REG_PAIR_ADIW(\rd,\rc) && (__w1n(\val) < 64)
	adiw	\rc,__w1n(\val)
  .else
	subi	\rc,__b2(\val)
	sbci	\rd,__b3(\val)
  .endif
 .elseif __b3(\val) != 0
	subi	\rd,__b3(\val)
 .endif
.endm

.macro	addi32f rd,rc,rb,ra, val
	subi	\ra,__b0n(\val)
	sbci	\rb,__b1n(\val)
	sbci	\rc,__b2n(\val)
	sbci	\rd,__b3n(\val)
.endm

.macro	subi32f rd,rc,rb,ra, val
  .if __IS_REG_PAIR_ADIW(\rb,\ra) && (__w0(\val) < 64)
    sbiw	\ra,__w0(\val)
  .else
	subi	\ra,__b0(\val)
	sbci	\rb,__b1(\val)
  .endif
	sbci	\rc,__b2(\val)
	sbci	\rd,__b3(\val)
.endm

.macro	cpi32 rd,rc,rb,ra, val, temp=TEMP_REG
 .if (\val) == 0
  .if __IS_REG_PAIR_ADIW(\rb,\ra)
	sbiw	\ra,0
  .else
	subi	\ra,0
	sbci	\rb,0
  .endif
	sbci	\rc,0
	sbci	\rd,0
 .else
  .if __b1(\val) == __b0(\val)
	ldi		\temp,__b0(\val)
	cp		\ra,\temp
	cpc		\rb,\temp
  .elseif __IS_HIGH_REG(\ra)
	cpi		\ra,__b0(\val)
	ldi		\temp,__b1(\val)
	cpc		\rb,\temp
  .else
	ldi		\temp,__b0(\val)
	cp		\ra,\temp
	ldi		\temp,__b1(\val)
	cpc		\rb,\temp
  .endif
  .if __b2(\val) != __b1(\val)
	ldi		\temp,__b2(\val)
  .endif
	cpc		\rc,\temp
  .if __b3(\val) != __b2(\val)
	ldi		\temp,__b3(\val)
  .endif
	cpc		\rd,\temp
 .endif
.endm

.macro cpi32b rd,rc,rb,ra, val, zero=R1
 .if (__w0(\val) == 0) && __IS_REG_PAIR_ADIW(\rb,\ra)
	sbiw	\ra,0
 .else
  .if __b0(\val) == 0
	cp		\ra,\zero
  .else
	cpi		\ra,__b0(\val)
  .endif
	cpc		\rb,\zero
 .endif
	cpc		\rc,\zero
	cpc		\rd,\zero
.endm

.macro cpi32w rd,rc,rb,ra, val, temp,zero=R1
 .if __b0(\val) == 0
	cp		\ra,\zero
  .if __b1(\val) != 0
	ldi		\temp,__b1(\val)
  .endif
 .elseif __IS_HIGH_REG(\ra)
	cpi		\ra,__b0(\val)
  .if __b1(\val) != 0
	ldi		\temp,__b1(\val)
  .endif
 .else
	ldi		\temp,__b0(\val)
	cp		\ra,\temp
  .if (__b1(\val) != 0) && (__b1(\val) != __b0(\val))
	ldi		\temp,__b1(\val)
  .endif
 .endif
 .if __b1(\val) == 0
	cpc		\rb,\zero
 .else
	cpc		\rb,\temp
 .endif
 .if __b2(\val) == 0
	cpc		\rc,\zero
 .else
  .if __b2(\val) != __b1(\val)
	ldi		\temp,__b2(\val)
  .endif
	cpc		\rc,\temp
 .endif
 .if __b3(\val) == 0
	cpc		\rd,\zero
 .else
  .if __b3(\val) != __b2(\val)
	ldi		\temp,__b3(\val)
  .endif
	cpc		\rd,\temp
 .endif
.endm

; --------------------------------------------------------------------------------------------------
; Logic
; --------------------------------------------------------------------------------------------------

; WORD

.macro	andw th,tl, rh,rl
	and		\tl,\rl
	and		\th,\rh
.endm

.macro	orw th,tl, rh,rl
	or		\tl,\rl
	or		\th,\rh
.endm

.macro	eorw th,tl, rh,rl
	eor		\tl,\rl
	eor		\th,\rh
.endm

.macro	comw rh,rl
	com		\rl
	com		\rh
.endm

; DWORD

.macro	and32 td,tc,tb,ta, rd,rc,rb,ra
	and		\ta,\ra
	and		\tb,\rb
	and		\tc,\rc
	and		\td,\rd
.endm

.macro	or32 td,tc,tb,ta, rd,rc,rb,ra
	or		\ta,\ra
	or		\tb,\rb
	or		\tc,\rc
	or		\td,\rd
.endm

.macro	eor32 td,tc,tb,ta, rd,rc,rb,ra
	eor		\ta,\ra
	eor		\tb,\rb
	eor		\tc,\rc
	eor		\td,\rd
.endm

.macro	com32 rd,rc,rb,ra
	com		\ra
	com		\rb
	com		\rc
	com		\rd
.endm

; --------------------------------------------------------------------------------------------------
; Logic (immediate)
; --------------------------------------------------------------------------------------------------

; WORD

.macro	andiw rh,rl, val
 .if __b0(\val) != 0xff
	andi	\rl,__b0(\val)
 .endif
 .if __b1(\val) != 0xff
	andi	\rh,__b1(\val)
 .endif
.endm

.macro	oriw rh,rl, val
 .if __b0(\val) != 0
	ori		\rl,__b0(\val)
 .endif
 .if __b1(\val) != 0
	ori		\rh,__b1(\val)
 .endif
.endm

; DWORD

.macro	andi32 rd,rc,rb,ra, val
 .if __b0(\val) != 0xff
	andi	\ra,__b0(\val)
 .endif
 .if __b1(\val) != 0xff
	andi	\rb,__b1(\val)
 .endif
 .if __b2(\val) != 0xff
	andi	\rc,__b2(\val)
 .endif
 .if __b3(\val) != 0xff
	andi	\rd,__b3(\val)
 .endif
.endm

.macro	ori32 rd,rc,rb,ra, val
 .if __b0(\val) != 0
	ori		\ra,__b0(\val)
 .endif
 .if __b1(\val) != 0
	ori		\rb,__b1(\val)
 .endif
 .if __b2(\val) != 0
	ori		\rc,__b2(\val)
 .endif
 .if __b3(\val) != 0
	ori		\rd,__b3(\val)
 .endif
.endm

; --------------------------------------------------------------------------------------------------
; Shift
; --------------------------------------------------------------------------------------------------

; BYTE

.macro __lsl rs,n=1
  .rept \n
	lsl		\rs
  .endr
.endm

.macro __lsr rs,n=1
  .rept \n
	lsr		\rs
  .endr
.endm

.macro __asr rs,n=1
  .rept \n
	asr		\rs
  .endr
.endm

.macro __rol rs,n=1
  .rept \n
	rol		\rs
  .endr
.endm

.macro __ror rs,n=1
  .rept \n
	ror		\rs
  .endr
.endm

; WORD

.macro	lslw rh,rl, n=1
 .if ((\n) & 15) >= 8
	mov		\rh,\rl
	eor		\rl,\rl
  .rept ((\n) & 7)
	lsl		\rh
  .endr
 .else
  .rept ((\n) & 7)
	lsl		\rl
	rol		\rh
  .endr
 .endif
.endm

.macro	lsrw rh,rl, n=1
 .if ((\n) & 15) >= 8
	mov		\rl,\rh
	eor		\rh,\rh
  .rept ((\n) & 7)
	lsr		\rl
  .endr
 .else
  .rept ((\n) & 7)
	lsr		\rh
	ror		\rl
  .endr
 .endif
.endm

.macro	asrw rh,rl, n=1
 .if ((\n) & 15) >= 8
	mov		\rl,\rh
	eor		\rh,\rh
	sbrc	\rl,7
	dec		\rh
  .rept ((\n) & 7)
	asr		\rl
  .endr
 .else
  .rept ((\n) & 7)
	asr		\rh
	ror		\rl
  .endr
 .endif
.endm

.macro	lslwf rh,rl, n=1
  .rept \n
	lsl		\rl
	rol		\rh
  .endr
.endm

.macro	lsrwf rh,rl, n=1
  .rept \n
	lsr		\rh
	ror		\rl
  .endr
.endm

.macro	asrwf rh,rl, n=1
  .rept \n
	asr		\rh
	ror		\rl
  .endr
.endm

.macro	rolw rh,rl, n=1
  .rept \n
	rol		\rl
	rol		\rh
  .endr
.endm

.macro	rorw rh,rl, n=1
  .rept \n
	ror		\rh
	ror		\rl
  .endr
.endm

.macro __rotlw rh,rl, n, temp=TEMP_REG
  .if ((\n) & 0xf) >= 8
	mov		\temp,\rh
	mov		\rh,\rl
	mov		\rl,\temp
  .endif
  .rept ((\n) & 0x7)
	mov		\temp,\rh
	lsl		\temp
	rol		\rl
	rol		\rh
  .endr
.endm

.macro __rotrw rh,rl, n, temp=TEMP_REG
  .if ((\n) & 0xf) >= 8
	mov		\temp,\rl
	mov		\rl,\rh
	mov		\rh,\temp
  .endif
  .rept ((\n) & 0x7)
	mov		\temp,\rl
	lsr		\temp
	ror		\rh
	ror		\rl
  .endr
.endm

.macro rotlw rh,rl, n, temp=TEMP_REG
  .if ((\n) & 0x7) <= 4
	__rotlw \rh,\rl, \n, \temp
  .else
	__rotrw \rh,\rl, 16 - ((\n) & 0xf), \temp
  .endif
.endm

.macro rotrw rh,rl, n, temp=TEMP_REG
  .if ((\n) & 0x7) <= 4
	__rotrw \rh,\rl, \n, \temp
  .else
	__rotlw \rh,\rl, 16 - ((\n) & 0xf), \temp
  .endif
.endm

; DWORD

.macro	lsl32 rd,rc,rb,ra, n=1
 .if ((\n) & 31) >= 24
	mov		\rd,\ra
	eor		\rc,\rc
	eor		\rb,\rb
	eor		\ra,\ra
  .rept ((\n) & 7)
	lsl		\rd
  .endr
 .elseif ((\n) & 31) >= 16
	mov		\rd,\rb
	mov		\rc,\ra
	eor		\rb,\rb
	eor		\ra,\ra
  .rept ((\n) & 7)
	lsl		\rc
	rol		\rd
  .endr
 .elseif ((\n) & 31) >= 8
	mov		\rd,\rc
	mov		\rc,\rb
	mov		\rb,\ra
	eor		\ra,\ra
  .rept ((\n) & 7)
	lsl		\rb
	rol		\rc
	rol		\rd
  .endr
 .else
  .rept ((\n) & 7)
	lsl		\ra
	rol		\rb
	rol		\rc
	rol		\rd
  .endr
 .endif
.endm

.macro	lsr32 rd,rc,rb,ra, n=1
 .if ((\n) & 31) >= 24
	mov		\ra,\rd
	eor		\rb,\rb
	eor		\rc,\rc
	eor		\rd,\rd
  .rept ((\n) & 7)
	lsr		\ra
  .endr
 .elseif ((\n) & 31) >= 16
	mov		\ra,\rc
	mov		\rb,\rd
	eor		\rc,\rc
	eor		\rd,\rd
  .rept ((\n) & 7)
	lsr		\rb
	ror		\ra
  .endr
 .elseif ((\n) & 31) >= 8
	mov		\ra,\rb
	mov		\rb,\rc
	mov		\rc,\rd
	eor		\rd,\rd
  .rept ((\n) & 7)
	lsr		\rc
	ror		\rb
	ror		\ra
  .endr
 .else
  .rept ((\n) & 7)
	lsr		\rd
	ror		\rc
	ror		\rb
	ror		\ra
  .endr
 .endif
.endm

.macro	asr32 rd,rc,rb,ra, n=1
 .if ((\n) & 31) >= 24
	mov		\ra,\rd
	eor		\rb,\rb
	eor		\rc,\rc
	eor		\rd,\rd
	sbrs	\ra,7
	rjmp	.+6
	dec		\rb
	dec		\rc
	dec		\rd
  .rept ((\n) & 7)
	asr		\ra
  .endr
 .elseif ((\n) & 31) >= 16
	mov		\ra,\rc
	mov		\rb,\rd
	eor		\rc,\rc
	eor		\rd,\rd
	sbrs	\rb,7
	rjmp	.+4
	dec		\rc
	dec		\rd
  .rept ((\n) & 7)
	asr		\rb
	ror		\ra
  .endr
 .elseif ((\n) & 31) >= 8
	mov		\ra,\rb
	mov		\rb,\rc
	mov		\rc,\rd
	eor		\rd,\rd
	sbrc	\rc,7
	dec		\rd
  .rept ((\n) & 7)
	asr		\rc
	ror		\rb
	ror		\ra
  .endr
 .else
  .rept ((\n) & 7)
	asr		\rd
	ror		\rc
	ror		\rb
	ror		\ra
  .endr
 .endif
.endm

.macro	lsl32f rd,rc,rb,ra, n=1
  .rept \n
	lsl		\ra
	rol		\rb
	rol		\rc
	rol		\rd
  .endr
.endm

.macro	lsr32f rd,rc,rb,ra, n=1
  .rept \n
	lsr		\rd
	ror		\rc
	ror		\rb
	ror		\ra
  .endr
.endm

.macro	asr32f rd,rc,rb,ra, n=1
  .rept \n
	asr		\rd
	ror		\rc
	ror		\rb
	ror		\ra
  .endr
.endm

.macro	rol32 rd,rc,rb,ra, n=1
  .rept \n
	rol		\ra
	rol		\rb
	rol		\rc
	rol		\rd
  .endr
.endm

.macro	ror32 rd,rc,rb,ra, n=1
  .rept \n
	ror		\rd
	ror		\rc
	ror		\rb
	ror		\ra
  .endr
.endm

.macro __rotl32 rd,rc,rb,ra, n, temp=TEMP_REG
  .if ((\n) & 0x1f) >= 24
	mov		\temp,\ra
	mov		\ra,\rb
	mov		\rb,\rc
	mov		\rc,\rd
	mov		\rd,\temp
  .elseif ((\n) & 0x1f) >= 16
	mov		\temp,\rc
	mov		\rc,\ra
	mov		\ra,\temp
	mov		\temp,\rd
	mov		\rd,\rb
	mov		\rb,\temp
  .elseif ((\n) & 0x1f) >= 8
	mov		\temp,\rd
	mov		\rd,\rc
	mov		\rc,\rb
	mov		\rb,\ra
	mov		\ra,\temp
  .endif
  .rept ((\n) & 0x07)
	mov		\temp,\rd
	lsl		\temp
	rol		\ra
	rol		\rb
	rol		\rc
	rol		\rd
  .endr
.endm

.macro __rotr32 rd,rc,rb,ra, n, temp=TEMP_REG
  .if ((\n) & 0x1f) >= 24
	mov		\temp,\rd
	mov		\rd,\rc
	mov		\rc,\rb
	mov		\rb,\ra
	mov		\ra,\temp
  .elseif ((\n) & 0x1f) >= 16
	mov		\temp,\ra
	mov		\ra,\rc
	mov		\rc,\temp
	mov		\temp,\rb
	mov		\rb,\rd
	mov		\rd,\temp
  .elseif ((\n) & 0x1f) >= 8
	mov		\temp,\ra
	mov		\ra,\rb
	mov		\rb,\rc
	mov		\rc,\rd
	mov		\rd,\temp
  .endif
  .rept ((\n) & 0x07)
	mov		\temp,\ra
	lsr		\temp
	ror		\rd
	ror		\rc
	ror		\rb
	ror		\ra
  .endr
.endm

.macro rotl32 rd,rc,rb,ra, n, temp=TEMP_REG
  .if ((\n) & 0x7) <= 4
	__rotl32	\rd,\rc,\rb,\ra, \n, \temp
  .else
	__rotr32	\rd,\rc,\rb,\ra, 32 - ((\n) & 0x1f), \temp
  .endif
.endm

.macro rotr32 rd,rc,rb,ra, n, temp=TEMP_REG
  .if ((\n) & 0x7) <= 4
	__rotr32	\rd,\rc,\rb,\ra, \n, \temp
  .else
	__rotl32	\rd,\rc,\rb,\ra, 32 - ((\n) & 0x1f), \temp
  .endif
.endm

; --------------------------------------------------------------------------------------------------
; Move, load, store
; --------------------------------------------------------------------------------------------------

; BYTE

.macro __ld rd, ptrh,ptrl
  .if __IS_Z_PTR(\ptrh,\ptrl)
	ld		\rd,Z
  .elseif __IS_Y_PTR(\ptrh,\ptrl)
	ld		\rd,Y
  .elseif __IS_X_PTR(\ptrh,\ptrl)
	ld		\rd,X
  .else
	.error "ld: register pointer required (X, Y or Z)"
  .endif
.endm

.macro ldp rd, ptrh,ptrl
  .if __IS_Z_PTR(\ptrh,\ptrl)
	ld		\rd,Z+
  .elseif __IS_Y_PTR(\ptrh,\ptrl)
	ld		\rd,Y+
  .elseif __IS_X_PTR(\ptrh,\ptrl)
	ld		\rd,X+
  .else
	.error "ldp: register pointer required (X, Y or Z)"
  .endif
.endm

.macro mld rd, ptrh,ptrl
  .if __IS_Z_PTR(\ptrh,\ptrl)
	ld		\rd,-Z
  .elseif __IS_Y_PTR(\ptrh,\ptrl)
	ld		\rd,-Y
  .elseif __IS_X_PTR(\ptrh,\ptrl)
	ld		\rd,-X
  .else
	.error "mld: register pointer required (X, Y or Z)"
  .endif
.endm

.macro __ldd rd, ptrh,ptrl,disp
  .if __IS_Z_PTR(\ptrh,\ptrl)
	ldd		\rd,Z+\disp
  .elseif __IS_Y_PTR(\ptrh,\ptrl)
	ldd		\rd,Y+\disp
  .else
	.error "ldd: register pointer required (Y or Z)"
  .endif
.endm

.macro __lpm rd, ptrh,ptrl
  .if __IS_Z_PTR(\ptrh,\ptrl)
	lpm		\rd,Z
  .else
	.error "lpm: Z pointer required"
  .endif
.endm

.macro lpmp rd, ptrh,ptrl
  .if __IS_Z_PTR(\ptrh,\ptrl)
	lpm		\rd,Z+
  .else
	.error "lpmp: Z pointer required"
  .endif
.endm

.macro __st ptrh,ptrl, rr
  .if __IS_Z_PTR(\ptrh,\ptrl)
	st		Z,\rr
  .elseif __IS_Y_PTR(\ptrh,\ptrl)
	st		Y,\rr
  .elseif __IS_X_PTR(\ptrh,\ptrl)
	st		X,\rr
  .else
	.error "st: register pointer required (X, Y or Z)"
  .endif
.endm

.macro stp ptrh,ptrl, rr
  .if __IS_Z_PTR(\ptrh,\ptrl)
	st		Z+,\rr
  .elseif __IS_Y_PTR(\ptrh,\ptrl)
	st		Y+,\rr
  .elseif __IS_X_PTR(\ptrh,\ptrl)
	st		X+,\rr
  .else
	.error "stp: register pointer required (X, Y or Z)"
  .endif
.endm

.macro mst ptrh,ptrl, rr
  .if __IS_Z_PTR(\ptrh,\ptrl)
	st		-Z,\rr
  .elseif __IS_Y_PTR(\ptrh,\ptrl)
	st		-Y,\rr
  .elseif __IS_X_PTR(\ptrh,\ptrl)
	st		-X,\rr
  .else
	.error "mst: register pointer required (X, Y or Z)"
  .endif
.endm

.macro __std ptrh,ptrl,disp, rr
  .if __IS_Z_PTR(\ptrh,\ptrl)
	std		Z+\disp,\rr
  .elseif __IS_Y_PTR(\ptrh,\ptrl)
	std		Y+\disp,\rr
  .else
	.error "std: register pointer required (Y or Z)"
  .endif
.endm

; WORD

.macro	ldiw rh,rl, val
	ldi		\rl,lo8(\val)
	ldi		\rh,hi8(\val)
.endm

.macro	ldsw rh,rl, addr
	lds		\rl,(\addr)+0
	lds		\rh,(\addr)+1
.endm

.macro	ldpw rh,rl, ptrh,ptrl
	ldp		\rl,\ptrh,\ptrl
	ldp		\rh,\ptrh,\ptrl
.endm

.macro	mldw rh,rl, ptrh,ptrl
	mld		\rl,\ptrh,\ptrl
	mld		\rh,\ptrh,\ptrl
.endm

.macro	lddw rh,rl, ptrh,ptrl,disp
	__ldd	\rl,\ptrh,\ptrl,\disp+0
	__ldd	\rh,\ptrh,\ptrl,\disp+1
.endm

.macro lpmpw rh,rl, ptrh,ptrl
	lpmp	\rl,\ptrh,\ptrl
	lpmp	\rh,\ptrh,\ptrl
.endm

.macro	stsw addr, rh,rl
	sts		\addr+0,\rl
	sts		\addr+1,\rh
.endm

.macro	stpw ptrh,ptrl, rh,rl
	stp		\ptrh,\ptrl,\rl
	stp		\ptrh,\ptrl,\rh
.endm

.macro	mstw ptrh,ptrl, rh,rl
	mst		\ptrh,\ptrl,\rl
	mst		\ptrh,\ptrl,\rh
.endm

.macro	stdw ptrh,ptrl,disp, rh,rl
	__std	\ptrh,\ptrl,\disp+0,\rl
	__std	\ptrh,\ptrl,\disp+1,\rh
.endm

; DWORD

.macro	ldi32 rd,rc,rb,ra, val
	ldi		\ra,__b0(\val)
	ldi		\rb,__b1(\val)
  .if (__w1(\val) == __w0(\val)) && __IS_REG_PAIR(\rd,\rc) && __IS_REG_PAIR(\rb,\ra)
	movw	\rc,\ra
  .else
	ldi		\rc,__b2(\val)
	ldi		\rd,__b3(\val)
  .endif
.endm

.macro	lds32 rd,rc,rb,ra, addr
	lds		\ra,\addr+0
	lds		\rb,\addr+1
	lds		\rc,\addr+2
	lds		\rd,\addr+3
.endm

.macro	ldp32 rd,rc,rb,ra, ptrh,ptrl
	ldp		\ra,\ptrh,\ptrl
	ldp		\rb,\ptrh,\ptrl
	ldp		\rc,\ptrh,\ptrl
	ldp		\rd,\ptrh,\ptrl
.endm

.macro	ldd32 rd,rc,rb,ra, ptrh,ptrl,disp
	__ldd	\ra,\ptrh,\ptrl,\disp+0
	__ldd	\rb,\ptrh,\ptrl,\disp+1
	__ldd	\rc,\ptrh,\ptrl,\disp+2
	__ldd	\rd,\ptrh,\ptrl,\disp+3
.endm

.macro	lpmp32 rd,rc,rb,ra, ptrh,ptrl
	lpmp	\ra,\ptrh,\ptrl
	lpmp	\rb,\ptrh,\ptrl
	lpmp	\rc,\ptrh,\ptrl
	lpmp	\rd,\ptrh,\ptrl
.endm

.macro	sts32 addr, rd,rc,rb,ra
	sts		\addr+0,\ra
	sts		\addr+1,\rb
	sts		\addr+2,\rc
	sts		\addr+3,\rd
.endm

.macro	stp32 ptrh,ptrl, rd,rc,rb,ra
	stp		\ptrh,\ptrl,\ra
	stp		\ptrh,\ptrl,\rb
	stp		\ptrh,\ptrl,\rc
	stp		\ptrh,\ptrl,\rd
.endm

.macro	mst32 ptrh,ptrl, rd,rc,rb,ra
	mst		\ptrh,\ptrl,\ra
	mst		\ptrh,\ptrl,\rb
	mst		\ptrh,\ptrl,\rc
	mst		\ptrh,\ptrl,\rd
.endm

.macro	std32 ptrh,ptrl,disp, rd,rc,rb,ra
	__std	\ptrh,\ptrl,\disp+0,\ra
	__std	\ptrh,\ptrl,\disp+1,\rb
	__std	\ptrh,\ptrl,\disp+2,\rc
	__std	\ptrh,\ptrl,\disp+3,\rd
.endm

; REVERSED WORD

.macro	ldswr rh,rl, addr
	lds		\rh,\addr+0
	lds		\rl,\addr+1
.endm

.macro	ldpwr rh,rl, ptrh,ptrl
	ldp		\rh,\ptrh,\ptrl
	ldp		\rl,\ptrh,\ptrl
.endm

.macro	mldwr rh,rl, ptrh,ptrl
	mld		\rh,\ptrh,\ptrl
	mld		\rl,\ptrh,\ptrl
.endm

.macro	lddwr rh,rl, ptrh,ptrl,disp
	__ldd	\rh,\ptrh,\ptrl,\disp+0
	__ldd	\rl,\ptrh,\ptrl,\disp+1
.endm

.macro	lpmpwr rh,rl, ptrh,ptrl
	lpmp	\rh,\ptrh,\ptrl
	lpmp	\rl,\ptrh,\ptrl
.endm

.macro	stswr addr, rh,rl
	sts		\addr+0,\rh
	sts		\addr+1,\rl
.endm

.macro	stpwr ptrh,ptrl, rh,rl
	stp		\ptrh,\ptrl,\rh
	stp		\ptrh,\ptrl,\rl
.endm

.macro	mstwr ptrh,ptrl, rh,rl
	mst		\ptrh,\ptrl,\rh
	mst		\ptrh,\ptrl,\rl
.endm

.macro	stdwr ptrh,ptrl,disp, rh,rl
	__std	\ptrh,\ptrl,\disp+0,\rh
	__std	\ptrh,\ptrl,\disp+1,\rl
.endm

; REVERSED DWORD

.macro	lds32r rd,rc,rb,ra, addr
	lds		\rd,\addr+0
	lds		\rc,\addr+1
	lds		\rb,\addr+2
	lds		\ra,\addr+3
.endm

.macro	ldp32r rd,rc,rb,ra, ptrh,ptrl
	ldp		\rd,\ptrh,\ptrl
	ldp		\rc,\ptrh,\ptrl
	ldp		\rb,\ptrh,\ptrl
	ldp		\ra,\ptrh,\ptrl
.endm

.macro	mld32r rd,rc,rb,ra, ptrh,ptrl
	mld		\rd,\ptrh,\ptrl
	mld		\rc,\ptrh,\ptrl
	mld		\rb,\ptrh,\ptrl
	mld		\ra,\ptrh,\ptrl
.endm

.macro	ldd32r rd,rc,rb,ra, ptrh,ptrl,disp
	__ldd	\rd,\ptrh,\ptrl,\disp+0
	__ldd	\rc,\ptrh,\ptrl,\disp+1
	__ldd	\rb,\ptrh,\ptrl,\disp+2
	__ldd	\ra,\ptrh,\ptrl,\disp+3
.endm

.macro	lpmp32r rd,rc,rb,ra, ptrh,ptrl
	lpmp	\rd,\ptrh,\ptrl
	lpmp	\rc,\ptrh,\ptrl
	lpmp	\rb,\ptrh,\ptrl
	lpmp	\ra,\ptrh,\ptrl
.endm

.macro	sts32r addr, rd,rc,rb,ra
	sts		\addr+0,\rd
	sts		\addr+1,\rc
	sts		\addr+2,\rb
	sts		\addr+3,\ra
.endm

.macro	stp32r ptrh,ptrl, rd,rc,rb,ra
	stp		\ptrh,\ptrl,\rd
	stp		\ptrh,\ptrl,\rc
	stp		\ptrh,\ptrl,\rb
	stp		\ptrh,\ptrl,\ra
.endm

.macro	mst32r ptrh,ptrl, rd,rc,rb,ra
	mst		\ptrh,\ptrl,\rd
	mst		\ptrh,\ptrl,\rc
	mst		\ptrh,\ptrl,\rb
	mst		\ptrh,\ptrl,\ra
.endm

.macro	std32r ptrh,ptrl,disp, rd,rc,rb,ra
	__std	\ptrh,\ptrl,\disp+0,\rd
	__std	\ptrh,\ptrl,\disp+1,\rc
	__std	\ptrh,\ptrl,\disp+2,\rb
	__std	\ptrh,\ptrl,\disp+3,\ra
.endm

; --------------------------------------------------------------------------------------------------
; Push/pop
; --------------------------------------------------------------------------------------------------

; BYTE

.macro __push rl:vararg
  .irp rs,\rl
	push	\rs
  .endr
.endm

.macro __pop rl:vararg
  .irp rt,\rl
	pop		\rt
  .endr
.endm

; WORD

.macro pushw rh,rl, rr:vararg
	push	\rl
	push	\rh
  .ifnb \rr
	pushw \rr
  .endif
.endm

.macro popw rh,rl, rr:vararg
	pop		\rh
	pop		\rl
  .ifnb \rr
	popw \rr
  .endif
.endm

; DWORD

.macro push32 rd,rc,rb,ra, rr:vararg
	push	\ra
	push	\rb
	push	\rc
	push	\rd
  .ifnb \rr
	push32 \rr
  .endif
.endm

.macro pop32 rd,rc,rb,ra, rr:vararg
	pop		\rd
	pop		\rc
	pop		\rb
	pop		\ra
  .ifnb \rr
	pop32 \rr
  .endif
.endm

; --------------------------------------------------------------------------------------------------
; Multiply
; --------------------------------------------------------------------------------------------------

; u16 = u8 * u8
.macro mulw t3,t2,t1,t0, a1,a0, b1,b0, zero
	mul		\a1,\b1
	__movw	\t3,\t2, R1,R0
	mul		\a0,\b0
	__movw	\t1,\t0, R1,R0
	mul		\a1,\b0
	add		\t1,R0
	adc		\t2,R1
	adc		\t3,\zero
	mul		\b1,\a0
	add		\t1,R0
	adc		\t2,R1
	adc		\t3,\zero
.endm

; s16 = s8 * s8
.macro mulsw t3,t2,t1,t0, a1,a0, b1,b0, zero
	muls	\a1,\b1
	__movw	\t3,\t2, R1,R0
	mul		\a0,\b0
	__movw	\t1,\t0, R1,R0
	mulsu	\a1,\b0
	sbc		\t3,\zero
	add		\t1,R0
	adc		\t2,R1
	adc		\t3,\zero
	mulsu	\b1,\a0
	sbc		\t3,\zero
	add		\t1,R0
	adc		\t2,R1
	adc		\t3,\zero
.endm

; u16 = (u8 * u8) << 1
.macro fmulw t3,t2,t1,t0, a1,a0, b1,b0, zero
	fmul	\a1,\b1
	__movw	\t3,\t2, R1,R0
	fmul	\a0,\b0
	__movw	\t1,\t0, R1,R0
	adc		\t2,\zero
	fmul	\a1,\b0
	sbc		\t3,\zero
	add		\t1,R0
	adc		\t2,R1
	adc		\t3,\zero
	fmul	\b1,\a0
	sbc		\t3,\zero
	add		\t1,R0
	adc		\t2,R1
	adc		\t3,\zero
.endm

; s16 = (s8 * s8) << 1
.macro fmulsw t3,t2,t1,t0, a1,a0, b1,b0, zero
	fmuls	\a1,\b1
	__movw	\t3,\t2, R1,R0
	fmul	\a0,\b0
	__movw	\t1,\t0, R1,R0
	adc		\t2,\zero
	fmulsu	\a1,\b0
	sbc		\t3,\zero
	add		\t1,R0
	adc		\t2,R1
	adc		\t3,\zero
	fmulsu	\b1,\a0
	sbc		\t3,\zero
	add		\t1,R0
	adc		\t2,R1
	adc		\t3,\zero
.endm

; --------------------------------------------------------------------------------------------------
; NOP
; --------------------------------------------------------------------------------------------------

.macro __nop n=1
	.rept (\n) / 2
		rjmp .+0
	.endr
	.if ((\n) % 2) != 0
		nop
	.endif
.endm

; --------------------------------------------------------------------------------------------------
; replace instructions

#define cbi		__cbi	// cbi	addr, n, temp=TEMP_REG
#define sbi		__sbi	// sbi	addr, n, temp=TEMP_REG
#define sbic	__sbic	// sbic	addr, n, temp=TEMP_REG
#define sbis	__sbis	// sbis	addr, n, temp=TEMP_REG
#define in		__in	// in	rd,addr
#define out		__out	// out	addr,rs
#define movw	__movw	// movw	th,tl, rh,rl
#define lsl		__lsl	// lsl	rs,n=1
#define lsr		__lsr	// lsr	rs,n=1
#define asr		__asr	// asr	rs,n=1
#define rol		__rol	// rol	rs,n=1
#define ror		__ror	// ror	rs,n=1
#define ld		__ld	// ld	rd, ptrh,ptrl
#define st		__st	// st	ptrh,ptrl, rr
#define ldd		__ldd	// ldd	rd, ptrh,ptrl,disp
#define lpm		__lpm	// lpm	rd, ptrh,ptrl
#define st		__st	// st	ptrh,ptrl, rr
#define std		__std	// std	ptrh,ptrl,disp, rr
#define push	__push	// push	rl:vararg
#define pop		__pop	// pop	rl:vararg
#define nop		__nop	// nop	n=1

#define adiw	addiw	// alternate name
#define sbiw	subiw	// alternate name

; --------------------------------------------------------------------------------------------------
; register renames
; --------------------------------------------------------------------------------------------------

; bytes
#ifndef ZH
  #define ZH		R31
  #define ZL		R30
  #define YH		R29
  #define YL		R28
  #define XH		R27
  #define XL		R26
#endif
#define EH			R25
#define EL			R24
#define DH			R23
#define DL			R22
#define CH			R21
#define CL			R20
#define BH			R19
#define BL			R18
#define AH			R17
#define AL			R16
#define T7H			R15
#define T7L			R14
#define T6H			R13
#define T6L			R12
#define T5H			R11
#define T5L			R10
#define T4H			R9
#define T4L			R8
#define T3H			R7
#define T3L			R6
#define T2H			R5
#define T2L			R4
#define T1H			R3
#define T1L			R2
#define T0H			R1
#define T0L			R0

; words
#define Z			ZH,ZL
#define Y			YH,YL
#define X			XH,XL
#define E			EH,EL
#define D			DH,DL
#define C			CH,CL
#define B			BH,BL
#define A			AH,AL
#define T7			T7H,T7L
#define T6			T6H,T6L
#define T5			T5H,T5L
#define T4			T4H,T4L
#define T3			T3H,T3L
#define T2			T2H,T2L
#define T1			T1H,T1L
#define T0			T0H,T0L

; dwords
#define ZX			Z,X
#define ZY			Z,Y
#define YX			Y,X
#define XE			X,E
#define ED			E,D
#define DC			D,C
#define CB			C,B
#define BA			B,A
#define T76			T7,T6
#define T65			T6,T5
#define T54			T5,T4
#define T43			T4,T3
#define T32			T3,T2
#define T21			T2,T1
#define T10			T1,T0

; avr-gcc registers
#define CTEMP		T0L
#define CZERO		T0H
#define CZEROW		T0H,T0H

; --------------------------------------------------------------------------------------------------
