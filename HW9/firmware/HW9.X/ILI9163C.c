// functions to operate the ILI9163C on the PIC32
// adapted from https://github.com/sumotoy/TFT_ILI9163C/blob/master/TFT_ILI9163C.cpp

// pin connections:
// VCC - 3.3V
// GND - GND
// CS - B7
// RESET - 3.3V
// A0 - B15
// SDA - A1
// SCK - B14
// LED - 3.3V

// B8 is turned into SDI1 but is not used or connected to anything

#include <xc.h>                     //  processor SFR definitions
#include <sys/attribs.h>            //  __ISR macro
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ILI9163C.h"

void SPI1_init() {
	SDI1Rbits.SDI1R = 0b0100; // B8 is SDI1
    RPA1Rbits.RPA1R = 0b0011; // A1 is SDO1
    TRISBbits.TRISB7 = 0; // SS is B7
    LATBbits.LATB7 = 1; // SS starts high

    // A0 / DAT pin
    ANSELBbits.ANSB15 = 0;
    TRISBbits.TRISB15 = 0;
    LATBbits.LATB15 = 0;
	
	SPI1CON = 0; // turn off the spi module and reset it
    SPI1BUF; // clear the rx buffer by reading from it
    SPI1BRG = 1; // baud rate to 12 MHz [SPI1BRG = (48000000/(2*desired))-1]
    SPI1STATbits.SPIROV = 0; // clear the overflow bit
    SPI1CONbits.CKE = 1; // data changes when clock goes from hi to lo (since CKP is 0)
    SPI1CONbits.MSTEN = 1; // master operation
    SPI1CONbits.ON = 1; // turn on spi1
}

unsigned char spi_io(unsigned char o) {
  SPI1BUF = o;
  while(!SPI1STATbits.SPIRBF) { // wait to receive the byte
    ;
  }
  return SPI1BUF;
}

void LCD_command(unsigned char com) {
    LATBbits.LATB15 = 0; // DAT
    LATBbits.LATB7 = 0; // CS
    spi_io(com);
    LATBbits.LATB7 = 1; // CS
}

void LCD_data(unsigned char dat) {
    LATBbits.LATB15 = 1; // DAT
    LATBbits.LATB7 = 0; // CS
    spi_io(dat);
    LATBbits.LATB7 = 1; // CS
}

void LCD_data16(unsigned short dat) {
    LATBbits.LATB15 = 1; // DAT
    LATBbits.LATB7 = 0; // CS
    spi_io(dat>>8);
    spi_io(dat);
    LATBbits.LATB7 = 1; // CS
}

void LCD_init() {
    int time = 0;
    LCD_command(CMD_SWRESET);//software reset
    time = _CP0_GET_COUNT();
    while (_CP0_GET_COUNT() < time + 48000000/2/2) {} //delay(500);

	LCD_command(CMD_SLPOUT);//exit sleep
    time = _CP0_GET_COUNT();
	while (_CP0_GET_COUNT() < time + 48000000/2/200) {} //delay(5);

	LCD_command(CMD_PIXFMT);//Set Color Format 16bit
	LCD_data(0x05);
	time = _CP0_GET_COUNT();
	while (_CP0_GET_COUNT() < time + 48000000/2/200) {} //delay(5);

	LCD_command(CMD_GAMMASET);//default gamma curve 3
	LCD_data(0x04);//0x04
	time = _CP0_GET_COUNT();
	while (_CP0_GET_COUNT() < time + 48000000/2/1000) {} //delay(1);

	LCD_command(CMD_GAMRSEL);//Enable Gamma adj
	LCD_data(0x01);
	time = _CP0_GET_COUNT();
	while (_CP0_GET_COUNT() < time + 48000000/2/1000) {} //delay(1);

	LCD_command(CMD_NORML);

	LCD_command(CMD_DFUNCTR);
	LCD_data(0b11111111);
	LCD_data(0b00000110);

    int i = 0;
	LCD_command(CMD_PGAMMAC);//Positive Gamma Correction Setting
	for (i=0;i<15;i++){
		LCD_data(pGammaSet[i]);
	}

	LCD_command(CMD_NGAMMAC);//Negative Gamma Correction Setting
	for (i=0;i<15;i++){
		LCD_data(nGammaSet[i]);
	}

	LCD_command(CMD_FRMCTR1);//Frame Rate Control (In normal mode/Full colors)
	LCD_data(0x08);//0x0C//0x08
	LCD_data(0x02);//0x14//0x08
	time = _CP0_GET_COUNT();
	while (_CP0_GET_COUNT() < time + 48000000/2/1000) {} //delay(1);

	LCD_command(CMD_DINVCTR);//display inversion
	LCD_data(0x07);
	time = _CP0_GET_COUNT();
	while (_CP0_GET_COUNT() < time + 48000000/2/1000) {} //delay(1);

	LCD_command(CMD_PWCTR1);//Set VRH1[4:0] & VC[2:0] for VCI1 & GVDD
	LCD_data(0x0A);//4.30 - 0x0A
	LCD_data(0x02);//0x05
	time = _CP0_GET_COUNT();
	while (_CP0_GET_COUNT() < time + 48000000/2/1000) {} //delay(1);

	LCD_command(CMD_PWCTR2);//Set BT[2:0] for AVDD & VCL & VGH & VGL
	LCD_data(0x02);
	time = _CP0_GET_COUNT();
	while (_CP0_GET_COUNT() < time + 48000000/2/1000) {} //delay(1);

	LCD_command(CMD_VCOMCTR1);//Set VMH[6:0] & VML[6:0] for VOMH & VCOML
	LCD_data(0x50);//0x50
	LCD_data(99);//0x5b
	time = _CP0_GET_COUNT();
	while (_CP0_GET_COUNT() < time + 48000000/2/1000) {} //delay(1);

	LCD_command(CMD_VCOMOFFS);
	LCD_data(0);//0x40
	time = _CP0_GET_COUNT();
	while (_CP0_GET_COUNT() < time + 48000000/2/1000) {} //delay(1);

	LCD_command(CMD_CLMADRS);//Set Column Address
	LCD_data16(0x00);
    LCD_data16(_GRAMWIDTH);

	LCD_command(CMD_PGEADRS);//Set Page Address
	LCD_data16(0x00);
    LCD_data16(_GRAMHEIGH);

	LCD_command(CMD_VSCLLDEF);
	LCD_data16(0); // __OFFSET
	LCD_data16(_GRAMHEIGH); // _GRAMHEIGH - __OFFSET
	LCD_data16(0);

	LCD_command(CMD_MADCTL); // rotation
    LCD_data(0b00001000); // bit 3 0 for RGB, 1 for GBR, rotation: 0b00001000, 0b01101000, 0b11001000, 0b10101000

	LCD_command(CMD_DISPON);//display ON
	time = _CP0_GET_COUNT();
	while (_CP0_GET_COUNT() < time + 48000000/2/1000) {} //delay(1);

	LCD_command(CMD_RAMWR);//Memory Write
}

void LCD_drawPixel(unsigned short x, unsigned short y, unsigned short color) {
    // check boundary
    LCD_setAddr(x,y,x+1,y+1);
    LCD_data16(color);
}

void LCD_setAddr(unsigned short x0, unsigned short y0, unsigned short x1, unsigned short y1) {
    LCD_command(CMD_CLMADRS); // Column
    LCD_data16(x0);
	LCD_data16(x1);

	LCD_command(CMD_PGEADRS); // Page
	LCD_data16(y0);
	LCD_data16(y1);

	LCD_command(CMD_RAMWR); //Into RAM
}

void LCD_clearScreen(unsigned short color) {
    int i;
    LCD_setAddr(0,0,_GRAMWIDTH,_GRAMHEIGH);
		for (i = 0;i < _GRAMSIZE; i++){
			LCD_data16(color);
		}
}

void LCD_writeLetter(unsigned short x, unsigned short y, char letter, unsigned short color)   {
    int ii,jj;                                                  // ii = column count (5 columns), jj = bit count (8 bits)
    if((x+5) < 128 && (y+5) <128)  {                            //  check if there's enough screen space
        for(ii=0;ii<5;ii++)    {
            for(jj = 0;jj<8;jj++)  {
                if((ASCII[letter-0x20][ii] >> jj) & 0x01)   {   // if the current pixel is defined on
                    LCD_drawPixel(x+ii,y+jj,color);
                }
                else    {
                    LCD_drawPixel(x+ii,y+jj,BACKGROUND);
                }     
            }
        }
    }
    else    {
        return;
    }
}

void LCD_writeString(unsigned short x, unsigned short y, char *message, unsigned short color)    {
    int ii = 0;
    unsigned short xp = x;
    while(message[ii]) {
        LCD_writeLetter(xp,y,message[ii],color);
        ii++;
        xp+=5; 
    }
}

void LCD_progressBar(unsigned short x, unsigned short y, unsigned short len, unsigned short color)  {
    int i,j;
    if(len>0 && (x+len)<128)   {
        for(i=0; i<len-x; i++) {
            for(j=0; j<4; j++) {
                LCD_drawPixel(x+i,y+j,color);
            }
        }    
    }
}

void LCD_positiveX(char len, unsigned short color, unsigned short x)    {
    int i,j;
    unsigned short cl = CROSSLENGTH;
    
    for(i=0; i<cl; i++)  {
        for(j=0; j<4; j++)  {
            if(i<len)   {
                LCD_drawPixel(x-i,YCEN+j,color);
            }
            else{
                LCD_drawPixel(x-i,YCEN+j,BACKGROUND);
            }
        }
    }
    
    if (len>0)  {
    LCD_negativeX(0,BACKGROUND,63); 
    }
}

void LCD_negativeX(char len, unsigned short color, unsigned short x)    {
    int i,j;
    unsigned short cl = CROSSLENGTH;
    
    for(i=0; i<cl; i++)    {
        for(j=0; j<4; j++)  {
            if(i<len)   {
                LCD_drawPixel(x+i,YCEN+j,color);
            }
            else    {
                LCD_drawPixel(x+i,YCEN+j,BACKGROUND);
            }
        } 
    }
    
    if (len>0)  {
        LCD_positiveX(0,BACKGROUND,65);
    }
}

void LCD_positiveY(char len, unsigned short color, unsigned short y)    {
    int i,j;
    unsigned short cl = CROSSLENGTH;
    
    for(j=0; j<cl; j++) {
        for(i=0; i<4; i++)  {
            if (j<len)  {
                LCD_drawPixel(XCEN+i,y-j,color);
            }
            else    {
                LCD_drawPixel(XCEN+i,y-j,BACKGROUND);
            }
        }
    }
    
    if (len>0)  {
    LCD_negativeY(0,BACKGROUND,81);
    }
    
}

void LCD_negativeY(char len, unsigned short color, unsigned short y)    {
    int i,j;
    unsigned short cl= CROSSLENGTH;
    
    for(j=0; j<cl; j++) {
        for(i=0; i<4; i++)  {
            if (j<len)  {
                LCD_drawPixel(XCEN+i,y+j,color);
            }
            else    {
                LCD_drawPixel(XCEN+i,y+j,BACKGROUND);
            }
        }
    }
    
    if (len>0)  {
    LCD_positiveY(0,BACKGROUND,79);
    }
}

void LCD_drawCross(float xacc, float yacc, unsigned short color)    {
    
    // note: center of LCD is x = 64, y = 80
    
    char xlen = abs(xacc * 50.0);
    char ylen = abs(yacc * 50.0);
    if (xlen < 50)  {
        if (xacc > 0) {
            LCD_positiveX(xlen,color,64);
        }
        else    {
            LCD_negativeX(xlen,color,64);
        }
    }
    
    if (ylen <50)   {

        if(yacc > 0)  {
            LCD_positiveY(ylen,color,80);
        }
        else    {
            LCD_negativeY(ylen,color,80);
        }
    }
}
