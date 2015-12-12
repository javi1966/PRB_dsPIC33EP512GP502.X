#include "drv18B20.h"


//********************************************************************

//  Rutinas sensor DS18B20P

//*********************************************************************

bool initErrDS18B20() {
    TRISDQ = 0;
    DQ18B20 = 0;
    delay_us(500);

    TRISDQ = 1;
    delay_us(5);

    if (DQ18B20 == 0)
        return 0;

    delay_us(80);

    if (DQ18B20 == 1)
        return 0;

    delay_us(420);
    TRISDQ = 1;

    return 1;
}

//***************************************************************************

void initDS18B20() {
    TRISDQ = 0;
    DQ18B20 = 0;
    delay_us(500);
    TRISDQ = 1;
    delay_us(80);
    delay_us(420);
    TRISDQ = 1;
}
//**************************************************************

void enviaDS18B20(byte dato) {
    byte cnt, d;

    for (cnt = 0; cnt < 8; ++cnt) {
        d = dato & 0x01; //toma bit 0,LSB
        TRISDQ = 0;
        DQ18B20 = 0;
        delay_us(2);
        //output_bit(DQ18S20,shift_right(&dato,1,0));
        DQ18B20 = d;
        delay_us(60);
        TRISDQ = 1;
        delay_us(2);
        dato = dato >> 1;
    }

}

//****************************************************************

byte leeDS18B20() {
    byte cnt, dato = 0;
    byte d;

    for (cnt = 0; cnt < 8; ++cnt) {
        TRISDQ = 0;
        DQ18B20 = 0;
        delay_us(2);
        TRISDQ = 1;
        delay_us(8);
        //shift_right(&dato,1,input(DQ18S20));
        d = DQ18B20;
        // delay_us(50);
        dato = dato >> 1;
        if (d == 1)
            dato += 0x80;
        delay_us(120);
    }

    return (dato);
}

//********************************************************************

byte leeRAMDS18B20(byte pos) {

    byte dato[9];

    if (pos > 8)
        return 0;

    initDS18B20();
    enviaDS18B20(0xCC); //skip ROM
    enviaDS18B20(0xBE); //lee RAM

    dato[0] = leeDS18B20(); //Temp. LSB
    dato[1] = leeDS18B20(); //Temp. MSB
    dato[2] = leeDS18B20(); // TH limit
    dato[3] = leeDS18B20(); // TL limit
    dato[4] = leeDS18B20(); // CONFIG
    dato[5] = leeDS18B20(); // RES 0
    dato[6] = leeDS18B20(); // RES 1
    dato[7] = leeDS18B20(); // RES 2
    dato[8] = leeDS18B20(); // CRC

    return dato[pos];


}
//*****************************************************************************

byte ponResDS18B20(byte res) {

    int cfg;

    if ((res < 9) || (res > 12))
        res = 9;
    cfg = ((res - 9) << 5);


    if (!initErrDS18B20())
        return 99;



    enviaDS18B20(0xCC);
    enviaDS18B20(0x4E);
    enviaDS18B20(0b01111101);
    enviaDS18B20(0b11001001);
    enviaDS18B20(cfg);

    initDS18B20();
    enviaDS18B20(0xCC);
    enviaDS18B20(0x48);
    delay_ms(15);
    return (leeRAMDS18B20((4 & 0b01100000) >> 5));
}

//******************************************************************************

byte leeTempDS18B20(byte Resol) {

    byte iConfig, iDly;
    int iTemp;


    //ponResDS18B20(Resol);

    iConfig = ponResDS18B20(Resol); //((leeRAMDS18B20(4) & 0b01100000)>>5);



    if (!initErrDS18B20()) //si falla nos retornara 99 en display,debug.
        return (99);



    enviaDS18B20(0xCC); //skip ROM
    enviaDS18B20(0x44); //prepara conversion

    iDly = 1 << iConfig; //retardo medida


    while (iDly--)
        delay_ms(100);

    delay_ms(100);

    iTemp = leeRAMDS18B20(1);
    iTemp <<= 8;
    iTemp += leeRAMDS18B20(0);
    iTemp &= 0x07FF;

    return iTemp / 16;
}
