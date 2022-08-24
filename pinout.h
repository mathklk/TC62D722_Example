#ifndef _PINOUT_H
#define _PINOUT_H

#include <xc.h>

#define SIN_TRI    TRISAbits.TRISA4 // SIN = SDI
#define SIN_LAT    LATAbits.LATA4
#define SCK_TRI    TRISAbits.TRISA6 // SCK = CLK
#define SCK_LAT    LATAbits.LATA6
#define TRANS_TRI  TRISAbits.TRISA9 // TRANS = LE
#define TRANS_LAT  LATAbits.LATA9
#define PWMCLK_TRI TRISAbits.TRISA7 // PWMCLK = OE
#define PWMCLK_LAT LATAbits.LATA7

#endif // _PINOUT_H