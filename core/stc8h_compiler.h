#ifndef STC8H_COMPILER_H
#define STC8H_COMPILER_H

#if defined(__SDCC)

#define STC8H_SDCC 1

#define STC8H_DATA __data
#define STC8H_IDATA __idata
#define STC8H_PDATA __pdata
#define STC8H_XDATA __xdata
#define STC8H_CODE __code
#define STC8H_BIT __bit

#define STC8H_SFR(name, addr) __sfr __at(addr) name
#define STC8H_SBIT(name, addr, bit) __sbit __at((addr) + (bit)) name
#define STC8H_SFRX(addr) (*((volatile STC8H_XDATA unsigned char *)(addr)))
#define STC8H_SFR16X(addr) (*((volatile STC8H_XDATA unsigned int *)(addr)))

#define STC8H_INTERRUPT(name, vector) void name(void) __interrupt(vector)
#define STC8H_INTERRUPT_USING(name, vector, reg_bank) void name(void) __interrupt(vector)
#define STC8H_NOP() __asm nop __endasm

#elif defined(__C51__) || defined(__CX51__)

#define STC8H_KEIL 1

#include <intrins.h>

#define STC8H_DATA data
#define STC8H_IDATA idata
#define STC8H_PDATA pdata
#define STC8H_XDATA xdata
#define STC8H_CODE code
#define STC8H_BIT bit

#define STC8H_SFR(name, addr) sfr name = addr
#define STC8H_SBIT(name, addr, bit) sbit name = addr ^ bit
#define STC8H_SFRX(addr) (*((volatile unsigned char xdata *)(addr)))
#define STC8H_SFR16X(addr) (*((volatile unsigned int xdata *)(addr)))

#define STC8H_INTERRUPT(name, vector) void name(void) interrupt vector
#define STC8H_INTERRUPT_USING(name, vector, reg_bank) void name(void) interrupt vector using reg_bank
#define STC8H_NOP() _nop_()

#else

#define STC8H_HOSTED 1

#define STC8H_DATA
#define STC8H_IDATA
#define STC8H_PDATA
#define STC8H_XDATA
#define STC8H_CODE const
#define STC8H_BIT unsigned char

#define STC8H_SFR(name, addr) extern volatile unsigned char name
#define STC8H_SBIT(name, addr, bit) extern volatile unsigned char name
#define STC8H_SFRX(addr) (*((volatile unsigned char *)(addr)))
#define STC8H_SFR16X(addr) (*((volatile unsigned int *)(addr)))

#define STC8H_INTERRUPT(name, vector) void name(void)
#define STC8H_INTERRUPT_USING(name, vector, reg_bank) void name(void)
#define STC8H_NOP() ((void)0)

#endif

#endif
