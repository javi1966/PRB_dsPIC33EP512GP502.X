/*
 * File:   adc.h
 * Author: t133643
 *
 * Created on 4 de septiembre de 2012, 12:16
 */

#ifndef ADC_H
#define	ADC_H

#include "GenericTypeDefs.h"
#include "delay.h"

#define SMP_BUFFER   100          //4 ciclos
#define SMPERIODO    49           //40000/128      // (FCY/FSAMPLE) - 1 ????
#define VDD          3.24
#define K_TRAFO      20.45
#define K_DIV        6.66
#define K_AD         VDD/1024
#define K_2R2        2.81         //2 x 1.414
#define K_AMP        6.8
#define K_CS60       1.61

#define TENSION      0        //AN0
#define CORRIENTE    1        //AN1


void initAD(void);
BOOL getBufferFull();
void setBufferFull(BOOL);
void startMedidas();
void stopMedidas();
WORD readADC(BYTE);
void detectaZeroCross();
WORD mideTension();
WORD mideCorriente();


#endif	/* ADC_H */

