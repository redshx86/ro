// -------------------------------------------------------------------------------------------------

#pragma once

#include "os_cfg.h"

// -------------------------------------------------------------------------------------------------
// Task flag registers.

#ifndef OS_TASK_REG_0
extern volatile uint8_t task_reg_0;
#define OS_TASK_REG_0				task_reg_0
#endif // !OS_TASK_REG_0

#if (OS_TASK_FLAG_COUNT > 8) && (!defined OS_TASK_REG_1)
extern volatile uint8_t task_reg_1;
#define OS_TASK_REG_1				task_reg_1
#endif // !OS_TASK_REG_1

#if (OS_TASK_FLAG_COUNT > 16) && (!defined OS_TASK_REG_2)
extern volatile uint8_t task_reg_2;
#define OS_TASK_REG_2				task_reg_2
#endif // !OS_TASK_REG_2

#if (OS_TASK_FLAG_COUNT > 24) && (!defined OS_TASK_REG_3)
extern volatile uint8_t task_reg_3;
#define OS_TASK_REG_3				task_reg_3
#endif // !OS_TASK_REG_3

// -------------------------------------------------------------------------------------------------
// Task bit manipulation.

#define _TASK_FLAG_SET(reg,bit)		reg |= 1<<(bit)
#define _TASK_FLAG_RST(reg,bit)		reg &= ~(1<<(bit))
#define _TASK_FLAG_TGL(reg,bit)		reg ^= (1<<(bit))
#define _TASK_FLAG_ISSET(reg,bit)	(reg & (1<<(bit)))

#define TASK_FLAG_SET(f)			_TASK_FLAG_SET(f)
#define TASK_FLAG_RST(f)			_TASK_FLAG_RST(f)
#define TASK_FLAG_TGL(f)			_TASK_FLAG_TGL(f)
#define TASK_FLAG_ISSET(f)			_TASK_FLAG_ISSET(f)

// -------------------------------------------------------------------------------------------------
// Individual flags.

#if OS_TASK_FLAG_COUNT >= 1
#define TASK_FLAG_0					OS_TASK_REG_0,0
#endif // OS_TASK_FLAG_COUNT >= 1

#if OS_TASK_FLAG_COUNT >= 2
#define TASK_FLAG_1					OS_TASK_REG_0,1
#endif // OS_TASK_FLAG_COUNT >= 2

#if OS_TASK_FLAG_COUNT >= 3
#define TASK_FLAG_2					OS_TASK_REG_0,2
#endif // OS_TASK_FLAG_COUNT >= 3

#if OS_TASK_FLAG_COUNT >= 4
#define TASK_FLAG_3					OS_TASK_REG_0,3
#endif // OS_TASK_FLAG_COUNT >= 4

#if OS_TASK_FLAG_COUNT >= 5
#define TASK_FLAG_4					OS_TASK_REG_0,4
#endif // OS_TASK_FLAG_COUNT >= 5

#if OS_TASK_FLAG_COUNT >= 6
#define TASK_FLAG_5					OS_TASK_REG_0,5
#endif // OS_TASK_FLAG_COUNT >= 6

#if OS_TASK_FLAG_COUNT >= 7
#define TASK_FLAG_6					OS_TASK_REG_0,6
#endif // OS_TASK_FLAG_COUNT >= 7

#if OS_TASK_FLAG_COUNT >= 8
#define TASK_FLAG_7					OS_TASK_REG_0,7
#endif // OS_TASK_FLAG_COUNT >= 8

#if OS_TASK_FLAG_COUNT >= 9
#define TASK_FLAG_8					OS_TASK_REG_1,0
#endif // OS_TASK_FLAG_COUNT >= 9

#if OS_TASK_FLAG_COUNT >= 10
#define TASK_FLAG_9					OS_TASK_REG_1,1
#endif // OS_TASK_FLAG_COUNT >= 10

#if OS_TASK_FLAG_COUNT >= 11
#define TASK_FLAG_10				OS_TASK_REG_1,2
#endif // OS_TASK_FLAG_COUNT >= 11

#if OS_TASK_FLAG_COUNT >= 12
#define TASK_FLAG_11				OS_TASK_REG_1,3
#endif // OS_TASK_FLAG_COUNT >= 12

#if OS_TASK_FLAG_COUNT >= 13
#define TASK_FLAG_12				OS_TASK_REG_1,4
#endif // OS_TASK_FLAG_COUNT >= 13

#if OS_TASK_FLAG_COUNT >= 14
#define TASK_FLAG_13				OS_TASK_REG_1,5
#endif // OS_TASK_FLAG_COUNT >= 14

#if OS_TASK_FLAG_COUNT >= 15
#define TASK_FLAG_14				OS_TASK_REG_1,6
#endif // OS_TASK_FLAG_COUNT >= 15

#if OS_TASK_FLAG_COUNT >= 16
#define TASK_FLAG_15				OS_TASK_REG_1,7
#endif // OS_TASK_FLAG_COUNT >= 16

#if OS_TASK_FLAG_COUNT >= 17
#define TASK_FLAG_16				OS_TASK_REG_2,0
#endif // OS_TASK_FLAG_COUNT >= 17

#if OS_TASK_FLAG_COUNT >= 18
#define TASK_FLAG_17				OS_TASK_REG_2,1
#endif // OS_TASK_FLAG_COUNT >= 18

#if OS_TASK_FLAG_COUNT >= 19
#define TASK_FLAG_18				OS_TASK_REG_2,2
#endif // OS_TASK_FLAG_COUNT >= 19

#if OS_TASK_FLAG_COUNT >= 20
#define TASK_FLAG_19				OS_TASK_REG_2,3
#endif // OS_TASK_FLAG_COUNT >= 20

#if OS_TASK_FLAG_COUNT >= 21
#define TASK_FLAG_20				OS_TASK_REG_2,4
#endif // OS_TASK_FLAG_COUNT >= 21

#if OS_TASK_FLAG_COUNT >= 22
#define TASK_FLAG_21				OS_TASK_REG_2,5
#endif // OS_TASK_FLAG_COUNT >= 22

#if OS_TASK_FLAG_COUNT >= 23
#define TASK_FLAG_22				OS_TASK_REG_2,6
#endif // OS_TASK_FLAG_COUNT >= 23

#if OS_TASK_FLAG_COUNT >= 24
#define TASK_FLAG_23				OS_TASK_REG_2,7
#endif // OS_TASK_FLAG_COUNT >= 24

#if OS_TASK_FLAG_COUNT >= 25
#define TASK_FLAG_24				OS_TASK_REG_3,0
#endif // OS_TASK_FLAG_COUNT >= 25

#if OS_TASK_FLAG_COUNT >= 26
#define TASK_FLAG_25				OS_TASK_REG_3,1
#endif // OS_TASK_FLAG_COUNT >= 26

#if OS_TASK_FLAG_COUNT >= 27
#define TASK_FLAG_26				OS_TASK_REG_3,2
#endif // OS_TASK_FLAG_COUNT >= 27

#if OS_TASK_FLAG_COUNT >= 28
#define TASK_FLAG_27				OS_TASK_REG_3,3
#endif // OS_TASK_FLAG_COUNT >= 28

#if OS_TASK_FLAG_COUNT >= 29
#define TASK_FLAG_28				OS_TASK_REG_3,4
#endif // OS_TASK_FLAG_COUNT >= 29

#if OS_TASK_FLAG_COUNT >= 30
#define TASK_FLAG_29				OS_TASK_REG_3,5
#endif // OS_TASK_FLAG_COUNT >= 30

#if OS_TASK_FLAG_COUNT >= 31
#define TASK_FLAG_30				OS_TASK_REG_3,6
#endif // OS_TASK_FLAG_COUNT >= 31

#if OS_TASK_FLAG_COUNT >= 32
#define TASK_FLAG_31				OS_TASK_REG_3,7
#endif // OS_TASK_FLAG_COUNT >= 32

// -------------------------------------------------------------------------------------------------
