#include <xc.h>
#include <math.h>

#include "adc.h"


WORD adcCntrVolt = 0;
WORD adcCntrCurr = 0;
BYTE bBufferFull = FALSE;
WORD inSamplesCurr[SMP_BUFFER];
WORD inSamplesVolt[SMP_BUFFER];
WORD inSamplesPow [SMP_BUFFER];
WORD tension[SMP_BUFFER];
WORD corriente[SMP_BUFFER];
UINT32 voltageAcc = 0;
UINT32 currentAcc = 0;
double powerAcc = 0;
double vinst, iinst;
int numSamples = 0;
UINT16 temp;

BOOL bInt1 = FALSE;

void initAD(void) {

    //_LATA3 = 0;
    //_TRISA3 = 1;
    _LATA0 = 0;
    _TRISA0 = 1;
    _LATA1 = 0;
    _TRISA1 = 1;
    _LATB2 = 0;
    _TRISB2 = 1;
    _LATB3 = 0;
    _TRISB3 = 1;


    //ANSELBbits.ANSB1 = 1;
    //  _ANSA3 = 1;
    _ANSA0 = 1;
    _ANSA1 = 1;
    AD1CON1 = 0x0000; // Turn off the A/D converter
    AD1CON1bits.FORM = 0; //INTEGER
    //AD1CON1bits.SSRC = 2; //TMR3
    AD1CON1bits.SSRC = 0b111; //auto convert
    AD1CON1bits.ASAM = 1; //auto
    //AD1CON1bits.ASAM = 0; //manual
    AD1CON3bits.ADCS = 0x0F; //8*TCY  = TAD
    AD1CON3bits.SAMC = 0x0F; //8*TAD   sample time
    //AD1CHS0bits.CH0SA = 0b11000; // Channel 0 positive input is the output of OA1/AN3
    AD1CHS0bits.CH0SA = 0b00000; //an1
    AD1CHS0bits.CH0NA = 0; //vss

    IFS0bits.AD1IF = 0; // Clear ADC Pair 0 Interrupt Flag
    IPC3bits.AD1IP = 0x02; // Set ADC Pair 0 Interrupt Priority (Level 1)
    IEC0bits.AD1IE = 0; // disable ADC Pair 0 Interrupt



    //Configura Timer 3
    //***   Timer3  50ns@40Mhz,fp=40M/2
    T3CONbits.TON = 0;
    TMR3 = 0x0000;
    PR3 = SMPERIODO; //625us / 16 =   39

    //Start Timer 2
    T3CONbits.TCKPS = 0b11; //   cada 50ns * 256 = 12.8 us

    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1;
    T3CONbits.TON = 0; //T3 ON


    AD1CON1bits.ADON = 1; // Turn off the A/D converter


}
//***********************************************************************

BOOL getBufferFull() {
    return bBufferFull;

}
//**********************************************************************

void setBufferFull(BOOL bBuff) {
    bBufferFull = bBuff;

}
//*********************************************************************+**

WORD maxValor(WORD arrayAD[], BYTE *counter) {
    WORD max;
    int i;

    BYTE index = 0;

    max = 0;
    for (i = 0; i < SMP_BUFFER - 1; i++) {

        if (arrayAD[i] > max) {
            max = arrayAD[i];
            index++;
        }
    }
    *counter = index;

    return max;


}
//************************************************************************

void startMedidas() {
    IEC0bits.T3IE = 1;
    T3CONbits.TON = 1; //T3 ON
    IFS0bits.AD1IF = 0;
    IEC0bits.AD1IE = 1; // Enable ADC  Interrupt
    AD1CON1bits.ADON = 1;
    bBufferFull = FALSE;

}
//************************************************************************

void stopMedidas() {
    IEC0bits.T3IE = 0;
    T3CONbits.TON = 0; //T2 OFF
    IEC0bits.AD1IE = 0; // disable ADC Pair 0 Interrupt
    AD1CON1bits.ADON = 0;
    adcCntrVolt = 0;

    adcCntrCurr = 0;
    numSamples = 0;
    bBufferFull = TRUE;


}

//******************************************************************

WORD readADC(BYTE canal) {

    AD1CHS0bits.CH0SA = canal;
    AD1CON1bits.SAMP = 1;

    while (!AD1CON1bits.DONE);
    return ADC1BUF0;

}
//***************************************************************

void detectaZeroCross() {
    BOOL sc = FALSE;
    WORD temp;

    while (sc == FALSE) {

        delay_us(100);
        temp = readADC(TENSION);
        if (temp == 512)
            sc = TRUE;

    }


}
//******************************************************************

WORD mideTension() {

    int i;

    WORD tension_hi = 0;


    for (i = 0; i < SMP_BUFFER; i++) {
        delay_us(200); //100*200us-> 20ms 50Hz
        tension[i] = readADC(TENSION);
        if (tension_hi < tension[i])
            tension_hi = tension[i];

    }

    if (tension_hi < 550)
        tension_hi = 511;


    return tension_hi;

}
//******************************************************************

WORD mideCorriente() {

    int i;

    WORD corriente_hi = 0;

    for (i = 0; i < 100; i++) {
        delay_us(200); //100*200us-> 20ms 50Hz
        corriente[i] = readADC(CORRIENTE);
        if (corriente_hi < corriente[i]) corriente_hi = corriente[i];

    }
    if (corriente_hi < 540)
        corriente_hi = 511;
    return corriente_hi;


}
//**********************************************************
WORD mideCorriente_v1() {
    
    WORD media;
    BYTE i;
    
    media=0;
    for(i=0;i<10;i++){
        
        media+=readADC(CORRIENTE);
        delay_us(50);
    }
    
     return media/10;
    
}

//************************************************************************

// Interrupt Service Routine code goes here

//void __attribute__((interrupt, no_auto_psv)) _ADC1Interrupt(void) {

void _ISRFAST _AD1Interrupt(void) {


    inSamplesVolt[adcCntrVolt++] = ADC1BUF0; //ReadADC10( 1 );

    inSamplesCurr[adcCntrCurr++] = ADC1BUF1;

    /*
    temp = ADC1BUF0;
    if (temp > 512) temp -= 512;
    else if (temp < 512) temp = 512 - temp;


    vinst = (temp * K_2R2 * K_AD) * K_TRAFO * K_DIV;
    voltageAcc += vinst * vinst; //pow(vinst, 2);
    
    temp = ADC1BUF1;
    if (temp > 515) temp -= 515;
    else if (temp < 515) temp = 515 - temp;

    iinst = (temp * K_2R2 * K_AD) * K_AMP;
    
    currentAcc
            += iinst * iinst;

    //powerAcc += iinst * vinst;
     */


    if (++numSamples >= SMP_BUFFER) {

        AD1CON1bits.ADON = 0;
        IEC0bits.AD1IE = 0; // disable ADC  Interrupt

        adcCntrVolt = 0;
        adcCntrCurr = 0;
        numSamples = 0;

        bBufferFull = TRUE;

    }
    IFS0bits.AD1IF = 0; // Clear ADC Pair 0 Interrupt Flag
}

void _ISR __attribute__((__no_auto_psv__)) _T3Interrupt(void) {


    static int i = 0;


    if (++i > 16) {

        i = 0;
    }


    IFS0bits.T3IF = 0;


}



