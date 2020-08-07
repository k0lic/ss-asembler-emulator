#include "processorcontext.h"

/* STATIC CONSTANTS */

const unsigned short ProcessorContext::ZERO = 1;
const unsigned short ProcessorContext::OVERFLOW = 2;
const unsigned short ProcessorContext::CARRY = 4;
const unsigned short ProcessorContext::NEGATIVE = 8;
const unsigned short ProcessorContext::TIMER = 8'192;
const unsigned short ProcessorContext::TERMINAL = 16'384;
const unsigned short ProcessorContext::INTERRUPT = 32'768;
const unsigned short ProcessorContext::ALL_FLAGS = 65'535;

/* CONSTRUCTOR */

ProcessorContext::ProcessorContext() 
{
	interruptLines[0] = interruptLines[1] = false;
	interruptTableEntries[0] = 2;
	interruptTableEntries[1] = 3;
}

/* GETTERS */

short ProcessorContext::getRegister(int regNum) 
{
	if (regNum == 15)
		return psw;
	else
		return reg[regNum];
}

short ProcessorContext::getRegisterLow(int regNum)
{
	short low = regNum == 15 ? psw & 0x00ff : reg[regNum] & 0x00ff;
	if (low & 0x80)
		low |= 0xff00;
	return low;
}

short ProcessorContext::getRegisterHigh(int regNum)
{
	short high = regNum == 15 ? psw >> 8 : reg[regNum] >> 8;
	if (high & 0x80)
		high |= 0xff00;
	return high;
}

unsigned short ProcessorContext::getPC() 
{
	return (unsigned short)reg[7];
}

unsigned short ProcessorContext::getSP()
{
	return (unsigned short)reg[6];
}

unsigned short ProcessorContext::getPSW() 
{
	return (unsigned short)psw;
}

bool ProcessorContext::getZero() 
{
	return psw & ProcessorContext::ZERO;
}

bool ProcessorContext::getOverflow() 
{
	return psw & ProcessorContext::OVERFLOW;
}

bool ProcessorContext::getCarry() 
{
	return psw & ProcessorContext::CARRY;
}

bool ProcessorContext::getNegative() 
{
	return psw & ProcessorContext::NEGATIVE;
}

bool ProcessorContext::getTimer() 
{
	return psw & ProcessorContext::TIMER;
}

bool ProcessorContext::getTerminal() 
{
	return psw & ProcessorContext::TERMINAL;
}

bool ProcessorContext::getInterrupt() 
{
	return psw & ProcessorContext::INTERRUPT;
}

bool ProcessorContext::interruptSignal(int interruptLineNum) 
{
	return interruptLines[interruptLineNum];
}

short ProcessorContext::getInterruptTableEntry(int num) 
{
	return interruptTableEntries[num];
}

/* SETTERS */

void ProcessorContext::setRegister(int regNum, short value) 
{
	if (regNum == 15)
		psw = value;
	else
		reg[regNum] = value;
}

void ProcessorContext::setRegisterLow(int regNum, short value)
{
	if (regNum == 15)
		psw = (psw & 0xff00) | (value & 0x00ff);
	else
		reg[regNum] = (reg[regNum] & 0xff00) | (value & 0x00ff);
}

void ProcessorContext::setRegisterHigh(int regNum, short value)
{
	if (regNum == 15)
		psw = (psw & 0x00ff) | (value << 8);
	else
		reg[regNum] = (reg[regNum] & 0x00ff) | (value << 8);
}

void ProcessorContext::setPC(unsigned short value) 
{
	reg[7] = value;
}

void ProcessorContext::incPC() 
{
	reg[7]++;
}

void ProcessorContext::setSP(unsigned short value)
{
	reg[6] = value;
}

void ProcessorContext::incSP()
{
	reg[6]++;
}

void ProcessorContext::decSP()
{
	reg[6]--;
}

void ProcessorContext::setPSW(unsigned short value) 
{
	psw = value;
}

void ProcessorContext::setZero(bool zero) 
{
	if (zero)
		psw |= ZERO;
	else
		psw &= ALL_FLAGS ^ ZERO;
}

void ProcessorContext::setOverflow(bool overflow) 
{
	if (overflow)
		psw |= OVERFLOW;
	else
		psw &= ALL_FLAGS ^ OVERFLOW;
}

void ProcessorContext::setCarry(bool carry) 
{
	if (carry)
		psw |= CARRY;
	else
		psw &= ALL_FLAGS ^ CARRY;
}

void ProcessorContext::setNegative(bool negative) 
{
	if (negative)
		psw |= NEGATIVE;
	else
		psw &= ALL_FLAGS ^ NEGATIVE;
}

void ProcessorContext::setTimer(bool timer) 
{
	if (timer)
		psw |= TIMER;
	else
		psw &= ALL_FLAGS ^ TIMER;
}

void ProcessorContext::setTerminal(bool terminal) 
{
	if (terminal)
		psw |= TERMINAL;
	else
		psw &= ALL_FLAGS ^ TERMINAL;
}

void ProcessorContext::setInterrupt(bool interrupt) 
{
	if (interrupt)
		psw |= INTERRUPT;
	else
		psw &= ALL_FLAGS ^ INTERRUPT;
}

void ProcessorContext::setInterruptSignal(int num, bool value) 
{
	interruptLines[num] = value;
}