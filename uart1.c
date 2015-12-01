
/*

UART1 Driver File for PIC24.

 ********************************************************************************
 FileName:        UART1.c
 Dependencies:    HardwareProfile.h
 Processor:       PIC24
 Compiler:        MPLAB C30
 Linker:          MPLAB LINK30
 Company:         Microchip Technology Incorporated

Author                Date      Comment
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
KO                 12-Feb-2008  Modified to use HardwareProfile.h
KO                 11-Oct-2006  v1.0
Anton Alkhimenok   18-Oct-2005
Anton Alkhimenok   17-Feb-2009  Added UART1Char2Hex(), UART1Hex2Char(),
                                                                          UART1ClearError(), UART1DataReceived()
PAT				   27-Jan-2010  Added UART1GetBaudError() for dynamic checking
                                                                of baud rate percentage error.

 ********************************************************************************
Software License Agreement

Microchip Technology Inc. ("Microchip") licenses to you the right to use, copy,
modify and distribute the software - including source code - only for use with
Microchip microcontrollers or Microchip digital signal controllers; provided
that no open source or free software is incorporated into the Source Code
without Microchip�s prior written consent in each instance.

The software is owned by Microchip and its licensors, and is protected under
applicable copyright laws.  All rights reserved.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, ANY CLAIMS BY THIRD PARTIES (INCLUDING
BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

 ********************************************************************************
 */

//#include "Compiler.h"
//#include "HardwareProfile.h"
#include "uart1.h"

//******************************************************************************
// Variables
//******************************************************************************

unsigned char txfifo[SER_BUFFER_SIZE];
volatile unsigned char txiptr, txoptr;
unsigned char rxfifo[SER_BUFFER_SIZE];
volatile unsigned char rxiptr, rxoptr;
unsigned char ser_tmp;

/*******************************************************************************
Function: UART1GetBaudError()

Precondition:
    None.

Overview:
    This routine checks the UART baud rate error percentage and returns it.

Input: None.

Output: Returns the baud rate error in percent.

 *******************************************************************************/
char UART1GetBaudError() {
    unsigned int errorPercent = 0;

    //errorPercent = ((BAUD_ERROR * 100 + BAUDRATE1 / 2) / BAUDRATE1);
    return (char) errorPercent;
}

/*******************************************************************************
Function: UART1GetChar()

Precondition:
    UART1Init must be called prior to calling this routine.

Overview:
    This routine waits for a byte to be received.  It then returns that byte.

Input: None.

Output: Byte received.

 *******************************************************************************/
char UART1GetChar() {
   // while(U1STAbits.URXDA == 0);
//	return U1RXREG;

     unsigned char c; //define

    while (UART1IsPressed()  == 0) //wait until a character is present
        continue; //

    c = rxfifo[rxoptr]; //get oldest character received
    ++rxoptr; //move the pointer to discard buffer
    rxoptr &= SER_FIFO_MASK; //if the pointer is at end, roll-it over.
    return c; //return it
}

/*******************************************************************************
Function: UART1Init()

Precondition: None.

Overview:
    This routine sets up the UART1 module.

Input: None.

Output: None.

Notes:
    Allow the peripheral to set the I/O pin directions.  If we set the TRIS
    bits manually, then when we disable the UART, the shape of the stop bit
    changes, and some terminal programs have problems.
 *******************************************************************************/
void UART1Init() {

    //setup UART
    U1BRG = 9;         //9@20mhz, 9@20.xxx=115200
    U1MODE = 0;
    U1MODEbits.BRGH = 0;
    U1STA = 0;
    IFS0bits.U1RXIF = 0;
    IEC0bits.U1RXIE = 1; //Enable RX interrupt

    U1MODEbits.UARTEN = 1;
    U1STAbits.UTXEN=1;
    RPINR18bits.U1RXR = 42;          //RB10 RX
    RPOR4bits.RP43R = 1;             //RB11 TX

    rxiptr = rxoptr = txiptr = txoptr = 0;

}

/*******************************************************************************
Function: UART1IsPressed()

Precondition:
    UART1Init must be called prior to calling this routine.

Overview:
    This routine checks to see if there is a new byte in UART reception buffer.

Input: None.

Output:
    0 : No new data received.
    1 : Data is in the receive buffer

 *******************************************************************************/
char UART1IsPressed() {
   // if (IFS0bits.U1RXIF == 1)
       // return 1;
   // return 0;
    if (U1STAbits.OERR) //error in reception?
    {
        U1STAbits.OERR = 0; //must clear the overrun error to keep uart receiving
        return 0; //	return no characters in buffer
    }
    return (rxiptr != rxoptr); //
}

//**********************************************************
unsigned char UART1RXRdy(void){
    return U1STAbits.URXDA;
}

/*******************************************************************************
Function: UART1PrintString( char *str )

Precondition:
    UART1Init must be called prior to calling this routine.

Overview:
    This function prints a string of characters to the UART.

Input: Pointer to a null terminated character string.

Output: None.

 *******************************************************************************/
void UART1PrintString(char *str) {
  

    while ( *str)
        UART1PutChar(*str++);

   
}

/*******************************************************************************
Function: UART1PutChar( char ch )

Precondition:
    UART1Init must be called prior to calling this routine.

Overview:
    This routine writes a character to the transmit FIFO, and then waits for the
    transmit FIFO to be empty.

Input: Byte to be sent.

Output: None.

 *******************************************************************************/
void UART1PutChar(char byte) {
   // while(U2STAbits.UTXBF);
   // U1TXREG = ch;
  while (U1STAbits.UTXBF)
        continue;
    txfifo[txiptr] = byte; //place character in buffer

    //increase ring input pointer and set it to zer0 if
    //it has rolled-it over.
    txiptr = (txiptr + 1) & SER_FIFO_MASK;

    IEC0bits.U1TXIE = 1; //enable interrupt driven

   // while (U1STAbits.TRMT == 0);
}

/*******************************************************************************
Function: UART1PutDec(unsigned char dec)

Precondition:
    UART1Init must be called prior to calling this routine.

Overview:
    This function converts decimal data into a string and outputs it to UART.

Input: Binary data.

Output: None.

 *******************************************************************************/
void UART1PutDec(unsigned char dec) {
    unsigned char res;
    unsigned char printed_already = 0;

    res = dec;

    if (res / 100) {
        UART1PutChar(res / 100 + '0');
        printed_already = 1;
    }
    res = res - (res / 100)*100;

    if ((res / 10) || (printed_already == 1)) {
        UART1PutChar(res / 10 + '0');
    }
    res = res - (res / 10)*10;

    UART1PutChar(res + '0');
}

/*******************************************************************************
Function: UART1PutHex

Precondition:
    UART1Init must be called prior to calling this routine.

Overview:
    This function converts hex data into a string and outputs it to UART.

Input: Binary data.

Output: None.

 *******************************************************************************/

const unsigned char CharacterArray[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

void UART1PutHex(int toPrint) {
    int printVar;

    printVar = toPrint;
    toPrint = (toPrint >> 4) & 0x0F;
    UART1PutChar(CharacterArray[toPrint]);

    toPrint = printVar & 0x0F;
    UART1PutChar(CharacterArray[toPrint]);

    return;
}

/*******************************************************************************
Function: UART1PutHexWord(unsigned int toPrint)

Precondition:
    UART1Init must be called prior to calling this routine.

Overview:
    This function converts hex data into a string and outputs it to UART.

Input: Binary data.

Output: None.

 *******************************************************************************/
#if defined( __C30__ ) || defined( __PIC32MX__ )

void UART1PutHexWord(unsigned int toPrint) {
    unsigned int printVar;

    printVar = (toPrint >> 12) & 0x0F;
    UART1PutChar(CharacterArray[printVar]);

    printVar = (toPrint >> 8) & 0x0F;
    UART1PutChar(CharacterArray[printVar]);

    printVar = (toPrint >> 4) & 0x0F;
    UART1PutChar(CharacterArray[printVar]);

    printVar = toPrint & 0x0F;
    UART1PutChar(CharacterArray[printVar]);

    return;
}

void UART1PutHexDWord(unsigned long toPrint) {
    unsigned long printVar;

    printVar = (toPrint >> 28) & 0x0F;
    UART1PutChar(CharacterArray[printVar]);

    printVar = (toPrint >> 24) & 0x0F;
    UART1PutChar(CharacterArray[printVar]);

    printVar = (toPrint >> 20) & 0x0F;
    UART1PutChar(CharacterArray[printVar]);

    printVar = (toPrint >> 16) & 0x0F;
    UART1PutChar(CharacterArray[printVar]);

    printVar = (toPrint >> 12) & 0x0F;
    UART1PutChar(CharacterArray[printVar]);

    printVar = (toPrint >> 8) & 0x0F;
    UART1PutChar(CharacterArray[printVar]);

    printVar = (toPrint >> 4) & 0x0F;
    UART1PutChar(CharacterArray[printVar]);

    printVar = toPrint & 0x0F;
    UART1PutChar(CharacterArray[printVar]);

    return;
}

#endif

/*********************************************************************
Function: char UART1Char2Hex(char ch)

PreCondition: none

Input: ASCII to be converted

Output: number

Side Effects: none

Overview: converts ASCII coded digit into number

Note: none

 ********************************************************************/
char UART1Char2Hex(char ch) {
    // Wrong char
    if (ch > 102)
        return 0;

    // From a to f
    if (ch > 96)
        return (ch - 87);

    // Wrong char
    if (ch > 70)
        return 0;

    // From A to F
    if (ch > 64)
        return (ch - 55);

    // Wrong char
    if (ch > 57)
        return 0;

    // From 0 - 9
    if (ch > 47)
        return (ch - 48);
    else
        // Wrong char
        return 0;
}

/*********************************************************************
Function: char UART1Hex2Char(char hex)

PreCondition: none

Input: number

Output: ASCII code

Side Effects: none

Overview: converts low nibble into ASCII coded digit

Note: none

 ********************************************************************/
char UART1Hex2Char(char hex) {
    char h;
    h = hex & 0x0f;
    // From 0xa to 0xf
    if (h > 9)
        return (h + 55);
    else
        return (h + 48);
}

/*********************************************************************
Function: void UART1ClrError(void)

PreCondition: none

Input: none

Output: character received

Side Effects: none

Overview: wait for character

Note: none

 ********************************************************************/
void UART1ClrError(void) {
    // Clear error flag
    if (U1STAbits.OERR)
        U1STAbits.OERR = 0;
}

//*********************************************************
void UART1PrintFloat(float f)
{
 float fenteros;
 float fdecimales;
 char buffer[8]={0,};
 int entero, decimal;

    fenteros = floor(f);
    fdecimales = f - fenteros;
    entero = (int)fenteros;
    decimal = (int)1000 * fdecimales;
    sprintf(buffer, "%d.%02i", entero, decimal);
    UART1PrintString(buffer);

}
//**********************************************************
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


