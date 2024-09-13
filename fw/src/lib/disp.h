// -------------------------------------------------------------------------------------------------

#pragma once

#ifndef __ASSEMBLER__
  #include <stdint.h>
  #include <avr/pgmspace.h>
#endif // __ASSEMBLER__

// -------------------------------------------------------------------------------------------------
// 7-segment glyphs
// AbCdEFGH1J_L_n0Pqr5tU___Y_
//   c    hi     o     u

// standard hex glyphs
#define SSEG_0			0x3F
#define SSEG_1			0x06
#define SSEG_2			0x5B
#define SSEG_3			0x4F
#define SSEG_4			0x66
#define SSEG_5			0x6D
#define SSEG_6			0x7D
#define SSEG_7			0x07
#define SSEG_8			0x7F
#define SSEG_9			0x6F
#define SSEG_A			0x77
#define SSEG_b			0x7C
#define SSEG_C			0x39
#define SSEG_d			0x5E
#define SSEG_E			0x79
#define SSEG_F			0x71

// letters
#define SSEG_G			0x3d
#define SSEG_H			0x76
#define SSEG_i			0x04
#define SSEG_J			0x1e
#define SSEG_L			0x38
#define SSEG_n			0x54
#define SSEG_P			0x73
#define SSEG_q			0x67
#define SSEG_r			0x50
#define SSEG_t			0x78
#define SSEG_U			0x3E
#define SSEG_Y			0x6E

// malformed letters
#define SSEG_xM			0x37

// extra lowercase letters
#define SSEG_c			0x58
#define SSEG_h			0x74
#define SSEG_o			0x5C
#define SSEG_u			0x1C

// symbols
#define SSEG_EMPTY		0x00
#define SSEG_MINUS		0x40
#define SSEG_DEG		0x63

#define SSEG_DP			0x80

// -------------------------------------------------------------------------------------------------
// 7-segment display

#define DISP_N			3

#ifndef __ASSEMBLER__
  extern uint8_t disp_buf[DISP_N];
  extern const uint8_t sseg_digit[10] PROGMEM;
#endif // __ASSEMBLER__

// -------------------------------------------------------------------------------------------------
