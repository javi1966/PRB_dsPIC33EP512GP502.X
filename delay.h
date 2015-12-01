/* 
 * File:   delay.h
 * Author: t133643
 *
 * Created on 30 de agosto de 2012, 19:20
 */

#ifndef DELAY_H
#define	DELAY_H

#include "GenericTypeDefs.h"

#define GetSystemClock()		(40000000ul)
#define GetInstructionClock()	(GetSystemClock()/2)
#define GetPeripheralClock()	(GetSystemClock()/2)


void delay_10us(DWORD dwCount);
void delay_ms(WORD ms);
void delay_us(unsigned int);


#endif	/* DELAY_H */

