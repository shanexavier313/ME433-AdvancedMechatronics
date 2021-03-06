#include<xc.h>              // processor SFR definitions
#include<sys/attribs.h>     // __ISR macro
#include<stdio.h>

// DEVCFG0
#pragma config DEBUG = OFF          // no debugging
#pragma config JTAGEN = OFF         // no jtag
#pragma config ICESEL = ICS_PGx1    // use PGED1 and PGEC1
#pragma config PWP = OFF            // no write protect
#pragma config BWP = OFF            // no boot write protect
#pragma config CP = OFF             // no code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL       // use primary oscillator with pll
#pragma config FSOSCEN = OFF        // turn off secondary oscillator
#pragma config IESO = OFF           // no switching clocks
#pragma config POSCMOD = HS         // high speed crystal mode
#pragma config OSCIOFNC = OFF       // free up secondary osc pins (unsure here)
#pragma config FPBDIV = DIV_1       // divide CPU freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD       // do not enable clock switch
#pragma config WDTPS = PS1048576    // slowest wdt (pretty sure this is slowest)
#pragma config WINDIS = OFF         // no wdt window
#pragma config FWDTEN = OFF         // wdt off by default
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the CPU clock to 48MHz
#pragma config FPLLIDIV = DIV_2     // divide input clock to be in range 4-5MHz (external clock is 8Mhz)
#pragma config FPLLMUL = MUL_24     // multiply clock after FPLLIDIV (get 96))
#pragma config FPLLODIV = DIV_2     // divide clock after FPLLMUL to get 48MHz (get 48))
#pragma config UPLLIDIV = DIV_2     // divider for the 8MHz input clock, then multiply by 12 to get 48MHz for USB
#pragma config UPLLEN = ON          // USB clock on

// DEVCFG3
#pragma config USERID = 0           // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF       // allow multiple reconfigurations
#pragma config IOL1WAY = OFF        // allow multiple reconfigurations
#pragma config FUSBIDIO = ON        // USB pins controlled by USB module
#pragma config FVBUSONIO = ON       // USB BUSON controlled by USB module

#include "i2c.h"                    //  i2c functions

int main() {
    __builtin_disable_interrupts();

    //  set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    //  0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    //  enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    //  disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    //  do your TRIS and LAT commands here
    /*
    TRISAbits.TRISA4 = 0;	//  A4 (LED) = output
	TRISBbits.TRISB4 = 1;	//  B4 (BUTTON) = input
	LATAbits.LATA4 = 1;		//  Turn on LED as default
    */
    
    //  I2C2 initialization 
    
    ANSELBbits.ANSB2 = 0;
    ANSELBbits.ANSB3 = 0;           //  turn off analog make SDA2 and SCL2
    
    i2c_master_setup();
    initExpander();
    
    __builtin_enable_interrupts();
  
    setExpander(0,0);               //  initialize GP0 as low  
    
    while(1)    {
        unsigned char r = getExpander();
        if ((r >> 7) & 0x01)    {
            setExpander(0,1);
        }
        else    {
            setExpander(0,0);
        }
    }
    return 0;
}