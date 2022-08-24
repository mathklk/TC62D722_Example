#include "pinout.h"
#include "characters.h"

#include <stdbool.h>

void initMc(void)
{
    // disable jtag so the RA4 output can be used
    DDPCONbits.JTAGEN = 0;
    
    // configure pins as outputs
    SIN_TRI = 0;
    SCK_TRI = 0;
    TRANS_TRI = 0;
    PWMCLK_TRI = 0;
    
    // initialize pins as digital low
    SIN_LAT = 0;
    SCK_LAT = 0;
    TRANS_LAT = 0;
    PWMCLK_LAT = 0;
}

void delayMs(const uint32_t nMs)
{
    int i;
    for (i = 0; i < (nMs*35u); ++i) {
        asm("nop");
    }
}

/**
 * Output a 16 bit value with SIN/CLK
 * @param inputBits
 */
void serialOutput16Bits(const uint16_t inputBits)
{
    uint16_t remainingBits = inputBits;
    int b;
    
    SIN_LAT = 0;
    int i;
    for (i = 0; i < 16; ++i) {
        b = remainingBits & 0x1;
        remainingBits = remainingBits >> 1;
        if (b) {
            SIN_LAT = 1;
        } else {
            SIN_LAT = 0;
        }
        SCK_LAT = 1;
        delayMs(1);
        SCK_LAT = 0;
        delayMs(1);
    }
    SIN_LAT = 0;
}

/**
 * I'm using two drivers in a daisy chain, so I have to fill all shift
 * registers before executing a command
 * 
 * In a real applicatin you would send different data to each IC
 * 
 * @param inputBits
 */
void serialOutputAllIcs(const uint16_t inputBits)
{
    int i;
    for (i = 0; i < 2; ++i) {
        serialOutput16Bits(inputBits);
    }
}

/**
 * Outputs the given number of pulses on SCK while TRANS="H"
 * @param nPulses
 */
void outputCommand(const int nPulses) {
    TRANS_LAT = 1;
    int i;
    for (i = 0; i < nPulses; i++) {
        delayMs(1);
        SCK_LAT = 1;
        delayMs(1);
        SCK_LAT = 0;
    }    
    delayMs(1);
    TRANS_LAT = 0;
}

/**
 * Cheap way to get a pwm signal on PWMCLK
 */
void outputPwmPulses(const int nPulses) {
    PWMCLK_LAT = 0;
    int i;
    for (i = 0; i < nPulses; i++) {
        asm("nop");
        PWMCLK_LAT = 1;
        asm("nop");
        PWMCLK_LAT = 0;
    }    
}

/**
 * Store the given data in the PWM reference registers of all ICs
 * @param data
 */
void outputPwmRegisters(const uint16_t data)
{
    int i;
    // repeat 16 times for all 16 OUT-pins of the IC
    for (i = 0; i < 16; ++i) {
        serialOutputAllIcs(data);
        outputCommand(0); // 0 = S0 (Store pwm value in PWM data regsister 1)
    }
    outputCommand(2); // 2 = S1 (Store data regsiter 1 in data regsiter 2 or 3)
}

/**
 * Turn the outputs on or off according to the 16 bit parameter
 * @param data
 */
void outputOnOffData(const uint16_t output16bit)
{
    serialOutputAllIcs(output16bit);
    outputCommand(7); // 7 = S2 = input of on/off data
}

/**
 * Do some initial configurations
 */
void outputStateSettingData(void)
{
    //                                    A6A4A2A0- - B0L0
    //                                    | | | | | | | |                     
    const uint16_t stateSettingData1 = 0b0000000000000000;
    //                                   | | | | | | | |
    //                                   A7A5A3A1- - B1H0
    
    //                                    D0F0I0K0N0- - -
    //                                    | | | | | | | | 
    const uint16_t stateSettingData2 = 0b1100100010000000;
    //                                   | | | | | | | |
    //                                   C0E0G0J0M0- - -
    // C0=1=Thermal shutdown inactive
    // D0=1=PWMCLK open detection inactive
    // G0=1=Asynchronous mode
    serialOutputAllIcs(stateSettingData1);
    outputCommand(13);
    serialOutputAllIcs(stateSettingData2);
    outputCommand(15); 
}

/**
 * Cycle between showing the letters A, B and C
 * (I use the driver to control a 16 segment display)
 */
void loop(void)
{
    outputOnOffData(ASCII_A);
    outputCommand(2); // 2 = S1;
    outputPwmPulses(0xFFFF);
    delayMs(100);
    outputOnOffData(ASCII_B);
    outputCommand(2); // 2 = S1;
    outputPwmPulses(0xFFFF);
    delayMs(100);
    outputOnOffData(ASCII_C);
    outputCommand(2); // 2 = S1;
    outputPwmPulses(0xFFFF);
    delayMs(100);
}

int main(void)
{
    initMc();
    outputStateSettingData();
    outputPwmRegisters(0x7FFF);
    while (1) {
        loop();
    }
}