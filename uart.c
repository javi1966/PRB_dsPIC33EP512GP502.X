#include "uart.h"


unsigned char txfifo[SER_BUFFER_SIZE];
volatile unsigned char txiptr, txoptr;
unsigned char rxfifo[SER_BUFFER_SIZE];
volatile unsigned char rxiptr, rxoptr;
unsigned char ser_tmp;


void initUART() {

    _TRISB10 = 1;    //RB7 -> RX
    _TRISB11 = 0;    //RB8 -> TX

    asm volatile ("mov #OSCCONL, w1  \n"
                "mov #0x46, w2     \n"
                "mov #0x57, w3     \n"
                "mov.b w2, [w1]    \n"
                "mov.b w3, [w1]    \n"
                "bclr OSCCON, #6");


    RPINR18bits.U1RXR = 10;          //RB7 RX
    RPOR4bits.RP43R = 0b000001;     //RB8 TX

    asm volatile ("mov #OSCCONL, w1  \n"
                "mov #0x46, w2     \n"
                "mov #0x57, w3     \n"
                "mov.b w2, [w1]    \n"
                "mov.b w3, [w1]    \n"
                "bset OSCCON, #6");


    U1BRG = 9;
    U1MODE = 0;
    U1MODEbits.BRGH = 0;
    U1STA = 0;

    
    U1STAbits.UTXEN = 1;
    IFS0bits.U1RXIF = 0;
    IEC0bits.U1RXIE=1;
    IEC0bits.U1TXIE=1;
    IPC2bits.U1RXIP=5;
    U1MODEbits.UARTEN = 1;
    rxiptr = rxoptr = txiptr = txoptr = 0;

}

//***********************************************************************
void putsUART(unsigned int *buffer)
{

  char * temp_ptr = (char *) buffer;

    /* transmit till NULL character is encountered */

    if(U1MODEbits.PDSEL == 3)        /* check if TX is 8bits or 9bits */
    {
        while(*buffer != '\0')
        {
            while(U1STAbits.UTXBF); /* wait if the buffer is full */
            U1TXREG = *buffer++;    /* transfer data word to TX reg */
        }
    }
    else
    {
        while(*temp_ptr != '\0')
        {
            while(U1STAbits.UTXBF);  /* wait if the buffer is full */
            U1TXREG = *temp_ptr++;   /* transfer data byte to TX reg */
        }
    }
}

//**************************************************************************
unsigned int getsUART1(unsigned int length,unsigned int *buffer,
                       unsigned int uart_data_wait)

{
    unsigned int wait = 0;
    char *temp_ptr = (char *) buffer;

    while(length)                         /* read till length is 0 */
    {
        while(!DataRdyUART())
        {
            if(wait < uart_data_wait)
                wait++ ;                  /*wait for more data */
            else
                return(length);           /*Time out- Return words/bytes to be read */
        }
        wait=0;
        if(U1MODEbits.PDSEL == 3)         /* check if TX/RX is 8bits or 9bits */
            *buffer++ = U1RXREG;          /* data word from HW buffer to SW buffer */
		else
            *temp_ptr++ = U1RXREG & 0xFF; /* data byte from HW buffer to SW buffer */

        length--;
    }

    return(length);                       /* number of data yet to be received i.e.,0 */
}

//******************************************************************************
char DataRdyUART(void)
{
    return(U1STAbits.URXDA);
}
//******************************************************************************
char BusyUART(void)
{
    return(!U1STAbits.TRMT);
}
//***************************************************************************
unsigned int ReadUART(void)
{
    if(U1MODEbits.PDSEL == 3)
        return (U1RXREG);
    else
        return (U1RXREG & 0xFF);
}
//***************************************************************************
void WriteUART(unsigned int data)
{
    if(U1MODEbits.PDSEL == 3)
        U1TXREG = data;
    else
        U1TXREG = data & 0xFF;
}
//**************************************************
char UART1IsPressed() {
    if(IFS0bits.U1RXIF == 1)
        return 1;
    return 0;
 
}
//*************************************************************
char UART1GetChar() {

    /*
      char Temp;

      while (IFS0bits.U1RXIF == 0);

      Temp = U1RXREG;
      IFS0bits.U1RXIF = 0;
      return Temp;*/

    unsigned char c; //define

    while (UART1IsPressed()  == 0) //wait until a character is present
        continue; //

    c = rxfifo[rxoptr]; //get oldest character received
    ++rxoptr; //move the pointer to discard buffer
    rxoptr &= SER_FIFO_MASK; //if the pointer is at end, roll-it over.
    return c; //return it
}

//*************************************************************
void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void) {
    U1TXREG = txfifo[txoptr];
    ++txoptr;
    txoptr &= SER_FIFO_MASK;
    if (txoptr == txiptr) IEC0bits.U1TXIE = 0;
}
//************************************************************************
void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void) {
    rxfifo[rxiptr] = U1RXREG;
    ser_tmp = (rxiptr + 1) & SER_FIFO_MASK;
    if (ser_tmp != rxoptr) rxiptr = ser_tmp;
    IFS0bits.U1RXIF = 0;
}

//***************************************************************










