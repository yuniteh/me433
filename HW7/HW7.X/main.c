#include <xc.h>           // processor SFR definitions
#include <sys/attribs.h>  // __ISR macro
#include <stdio.h>
#include "lcd.h"
#include "imu.h"

// DEVCFG0
#pragma config DEBUG = OFF // no debugging
#pragma config JTAGEN = OFF // no jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // no write protect
#pragma config BWP = OFF // no boot write protect
#pragma config CP = OFF // no code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // turn off secondary oscillator
#pragma config IESO = OFF // no switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // disable secondary osc
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // do not enable clock switch
#pragma config WDTPS = PS1 // use slowest wdt
#pragma config WINDIS = OFF // wdt no window mode
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz
#pragma config UPLLIDIV = DIV_2 // divider for the 8MHz input clock, then multiplied by 12 to get 48MHz for USB
#pragma config UPLLEN = ON // USB clock on

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations
#pragma config FUSBIDIO = ON // USB pins controlled by USB module
#pragma config FVBUSONIO = ON // USB BUSON controlled by USB module

int main() {

    __builtin_disable_interrupts();

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    TRISBbits.TRISB4 = 1;
    TRISAbits.TRISA4 = 0;

    ANSELBbits.ANSB2 = 0;
    ANSELBbits.ANSB3 = 0;
    init_imu();
    SPI1_init();
    LCD_init();
    __builtin_enable_interrupts();

    LCD_clearScreen(WHITE);
    unsigned char data[6];
    short out[3];
    char str[25];
    float acc[3];

    while (1) {
        _CP0_SET_COUNT(0);
        while (_CP0_GET_COUNT() < 4800000) {
            ;
        }
        i2c_read_multiple(ADD, 0x28, data, 6);
        int i;
        for (i = 0; i < 6; i += 2) {
            out[i / 2] = (short) data[i + 1] << 8 | data[i];
        }
        for (i = 0; i < 3; i++) {
            acc[i] = out[i]*.00061*4;
        }

        //        sprintf(str, "%hhu       ", data[1]);
        //        LCD_writeString(28, 32, str);
        //        sprintf(str, "%hhu       ", data[0]);
        //        LCD_writeString(28, 40, str);
//        sprintf(str, "%d       ", (int) acc[0]);
//        LCD_writeString(28, 48, str);
//        LCD_bar(28, 32, (int) 2 * acc[0]);
        LCD_barX(63, 63, (int) acc[0]);
        LCD_barY(63, 63, (int) acc[1]);
    }
}
