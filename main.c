/* 
 * File:   main.c
 * Author: t133643
 *
 * Created on 25 de octubre de 2014, 11:07
 */


#include <xc.h>
#include <stdlib.h>
#include "adc.h"
#include "uart1.h"
#include "drv18B20.h"

_FPOR(ALTI2C1_OFF & ALTI2C2_OFF);
_FWDT(PLLKEN_ON & FWDTEN_OFF);
_FOSCSEL(FNOSC_FRC & IESO_OFF);
_FGS(GWRP_OFF & GCP_OFF); //General Segment
_FICD(ICS_PGD2 & JTAGEN_OFF); // PGD3 for 28pin devices
// PGD2 for 44pin devices
_FOSC(FCKSM_CSECMD & OSCIOFNC_OFF); //XT W/PLL

//***************************************************************
#define LED                _LATB4
#define TRIS_LED           _TRISB4
#define VAL_MIN_VOLT_MAX   88
#define VAL_VREF           511

#define VAL_AREF           443
#define FACTOR_A           25.9

#define GetSystemClock()		(40000000ul)
#define GetInstructionClock()	(GetSystemClock()/2)
#define GetPeripheralClock()	(GetSystemClock()/2)

BOOL bVisu = FALSE;
extern WORD tension[SMP_BUFFER];
extern WORD corriente[SMP_BUFFER];
long medVAC = 0;
long medCorriente = 0;
WORD Tension;
float Corriente;
BYTE Temperatura;
char buffer[5] = {0};

//**********************************************************
void borraBuffer()
{
    int i;
    for (i=0;i<5;i++)
        buffer[i]='\0';
    
}
//***************************************************************
void enviaDatos(WORD *buffer) {
    int n;

    n = SMP_BUFFER;

    while (n--) {
        UART1PutHexWord(buffer[n]);
        UART1PutChar(',');
    }

}

/**************************************************************/
int main(int argc, char** argv) {

    
    char cmd;

    //The settings below set up the oscillator and PLL for 60 MIPS as
    //follows:
    //            Crystal Frequency  * (DIVISOR+2)
    // Fcy =     ---------------------------------
    //            2 * (PLLPOST+1) * (PLLPRE+2)
    // Crystal  = 8 MHz
    // Fosc		= 40 MHz
    // Fcy		= 20 MIPS
    //40Mhz
    PLLFBD = 18; // M=20
    CLKDIVbits.PLLPOST = 0; // N1=2
    CLKDIVbits.PLLPRE = 0; // N2=2
    __builtin_write_OSCCONH(0x01);
    __builtin_write_OSCCONL(0x01);

    while (OSCCONbits.COSC != 0b001);
    // Wait for PLL to lock
    while (OSCCONbits.LOCK != 1);


    TRIS_LED = 0;
    LED = 1;

    //configura OPAMP
    CM1CONbits.CON = 0;
    CM1CONbits.OPMODE = 1; //OPAMP 1 on
    CM1CONbits.COE = 1;     //salida OPAMP A/D exterior
    CVRCONbits.CVREN = 1;
    CVRCONbits.CVR2OE = 1; // 1/2 Vdd-VSS/2
    CM1CONbits.CON = 1;
    
    initAD();
    UART1Init();


    //Configura TMR1 FCY=(1/20000000)0.05us * 256 * 65536
    T1CONbits.TCKPS = 3; //256
    PR1 = 782; //781 * 12.8us = 10000us=10ms
    TMR1 = 0;
    IPC0bits.T1IP = 2; // Interrupt priority 2 (low)
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;
    T1CONbits.TON = 1;

    //Configura Timer 2 para lectura sensor temperatura.

    T2CONbits.TCKPS = 3; //256 FCY=(1/20000000)0.05us * 256 * 65536
    TMR2 = 0;
    IPC1bits.T2IP = 1;
    IFS0bits.T2IF = 0;
    IEC0bits.T2IE = 1;
    T2CONbits.TON = 1; 

    // Medida inicial **************************************************************
    Temperatura = leeTempDS18B20(9);
     
     //*************************************************************
    while (TRUE) {


        if (UART1IsPressed()) {

            cmd = UART1GetChar();

            switch (cmd) {
                case 'T': //dame tension
                case 't':
                    delay_ms(150);
                    borraBuffer();
                    if (medVAC)
                        Tension =( 237 * (medVAC - VAL_VREF)
                                / (VAL_VREF-VAL_MIN_VOLT_MAX));  
                    else
                        Tension = 0;

                    itoa(buffer, Tension, 10);
                    UART1PutChar(0xAA);
                    UART1PrintString(buffer);
                    UART1PutChar(0x55);

                    break;
                case 'C': //dame corriente
                case 'c':
                   
                   
                    Corriente=0.0;
                
                    delay_ms(500);  
                    Corriente = FACTOR_A  * (mideCorriente_v1() - VAL_AREF )/VAL_AREF;
                                         

                    UART1PutChar(0xAA);
                    UART1PrintFloat(Corriente);
                    UART1PutChar(0x55);
                    
                    break;

                case 'A': //dame buffer tension
                case 'a':
                    UART1PutChar(0xAA);
                    enviaDatos(tension);
                    UART1PutChar(0x55);
                    break;

                case 'B': //dame buffer corriente
                case 'b':
                    UART1PutChar(0xAA);
                    enviaDatos(corriente);
                    UART1PutChar(0x55);
                    break;

                case 'P': //dame buffer corriente
                case 'p':
                     borraBuffer();
                     Corriente=0.0;
                    LED = 0;
                    delay_ms(150);
                    LED = 1;
                    UART1PutChar(0xAA);
                    if (medVAC)
                        Tension =( 237 * (medVAC - VAL_VREF)/ (VAL_VREF-VAL_MIN_VOLT_MAX));  
                                
                    else
                        Tension = 0;
                    
                    //para ver en github
                    Corriente = FACTOR_A  * (mideCorriente_v1() - VAL_AREF )/VAL_AREF;   
                    
                    
                    itoa(buffer, Tension, 10);
                    UART1PrintString(buffer);
                    UART1PutChar('#');
                    UART1PrintFloat(Corriente);
                    UART1PutChar(0x55);
                    break;

                case 'K':
                case 'k':
                    borraBuffer();
                    //Temperatura = leeTempDS18B20(9);
                    itoa(buffer, Temperatura, 10);

                    UART1PutChar(0xAA);
                    UART1PrintString(buffer);
                    UART1PutChar(0x55);
                    break;
                default:
                    break;

            }

        }

    }//WHILE

    return (1);
}

//************************************************************************

void _ISR __attribute__((__no_auto_psv__)) _T1Interrupt(void) {

    static int count = 0;
    static int cnt = 0;

    if (++count > 100) { // 100 cada 10ms
        bVisu = 1;

        //  UART1PrintString("TEST\r\n");

        //  if (bVisu == TRUE) { //cada segundo

        detectaZeroCross();
        medVAC += mideTension();

        detectaZeroCross();
        medCorriente += mideCorriente();

        if (++cnt > 1) {
            medVAC /= 2;
            medCorriente /= 2;
        }

        if (cnt > 59) //  59->1 minuto ,60 sec.
        {
            //230 max. medido,761 convertidor - 523 =238
            // Tension = (230 * (medVAC - 511)) / 248;
            // Corriente = (8.29 * (medCorriente - 511)) / 185;
            cnt = 0;
            medVAC = 0;
            medCorriente = 0;
        }

        count = 0;
    }

    IFS0bits.T1IF = 0;

}
//*************************************************************************
void _ISR __attribute__((__no_auto_psv__)) _T2Interrupt(void) {
    
    static int cnt = 0;
    
    if(++cnt > 60) //aprox 1 min
    {
        Temperatura = leeTempDS18B20(9);
        cnt=0;
    }   
     IFS0bits.T2IF = 0;
}