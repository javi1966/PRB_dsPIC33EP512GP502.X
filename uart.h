/* 
 * File:   uart.h
 * Author: Javier
 *
 * Created on 16 de mayo de 2013, 18:11
 */

#ifndef UART_H
#define	UART_H

#include "xc.h"
#include "GenericTypeDefs.h"

#define SER_BUFFER_SIZE		64
#define SER_FIFO_MASK 		(SER_BUFFER_SIZE-1)




#define GetSystemClock()		(40000000ul)
#define GetInstructionClock()	(GetSystemClock()/2)
#define GetPeripheralClock()	(GetSystemClock()/2)

#define BAUD_RATE			9600

#define CLOSEST_UBRG_VALUE ((GetPeripheralClock()+8ul*BAUD_RATE)/16/BAUD_RATE-1)
#define BAUD_ACTUAL (GetPeripheralClock()/16/(CLOSEST_UBRG_VALUE+1))
#define BAUD_ERROR ((BAUD_ACTUAL > BAUD_RATE) ? BAUD_ACTUAL-BAUD_RATE : BAUD_RATE-BAUD_ACTUAL)
#define BAUD_ERROR_PRECENT	((BAUD_ERROR*100+BAUD_RATE/2)/BAUD_RATE)

/*
#if (BAUD_ERROR_PRECENT > 3)
     #warning UART frequency error is worse than 3%
#elif (BAUD_ERROR_PRECENT > 2)
    #warning UART frequency error is worse than 2%
#endif*/

void initUART(void);
void putsUART(unsigned int *buffer);
#define putrsUART(x) putsUART( (unsigned int *)x)
unsigned int getsUART1(unsigned int length, unsigned int *buffer,
        unsigned int uart_data_wait);
char DataRdyUART(void);
char BusyUART(void);
unsigned int ReadUART(void);
void WriteUART(unsigned int data);


#endif	/* UART_H */

