#include "delay.h"


void delay_10us(DWORD dwCount)
{
	volatile DWORD _dcnt;

	_dcnt = dwCount*((DWORD)(0.00001/(1.0/GetInstructionClock())/10));
	while(_dcnt--)
	{
		#if defined(__C32__)
			Nop();
			Nop();
			Nop();
		#endif
	}
}

//***************************************************************
void delay_ms(WORD ms)
{
    unsigned char i;
    while(ms--)
    {
        i=4;
        while(i--)
        {
            delay_10us(25);
        }
    }
}

//*************************************************
void delay_us(unsigned int delay){

    int i;
    for(i=0;i<delay;i++)
    {

        __asm__ volatile ("repeat #19");
        __asm__ volatile ("nop");
    }
}
