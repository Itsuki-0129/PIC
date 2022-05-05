/*
 * File:   newmain.c
 * Author: nagaoitsuki
 *
 * Created on May 5, 2022, 4:43 PM
 */



// PIC16F1938 Configuration Bit Settings

// 'C' source line config statements


// PIC16F1938 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = ECH       // Oscillator Selection (ECH, External Clock, High Power Mode (4-32 MHz): device clock supplied to CLKIN pin)
#pragma config WDTE = ON        // Watchdog Timer Enable (WDT enabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = ON       // MCLR Pin Function Select (MCLR/VPP pin function is MCLR)
#pragma config CP = OFF         // Flash Program Memory Code Protection (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Memory Code Protection (Data memory code protection is disabled)
#pragma config BOREN = ON       // Brown-out Reset Enable (Brown-out Reset enabled)
#pragma config CLKOUTEN = OFF   // Clock Out Enable (CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin)
#pragma config IESO = ON        // Internal/External Switchover (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enable (Fail-Safe Clock Monitor is enabled)

// CONFIG2
#pragma config WRT = OFF        // Flash Memory Self-Write Protection (Write protection off)
#pragma config VCAPEN = OFF     // Voltage Regulator Capacitor Enable (All VCAP pin functionality is disabled)
#pragma config PLLEN = ON       // PLL Enable (4x PLL enabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset Enable (Stack Overflow or Underflow will cause a Reset)
#pragma config BORV = LO        // Brown-out Reset Voltage Selection (Brown-out Reset Voltage (Vbor), low trip point selected.)
#pragma config LVP = ON         // Low-Voltage Programming Enable (Low-voltage programming enabled)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#define _XTAL_FREQ 8000000 //8MHz

void display(int number) {
    int kathode=0,dnum=0,kc[]={0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x27,0x7f,0x6f,0x00};
    kathode=1;
    for (int i=1;i<=4;i++) {
        switch (i) {
            case 1:
                dnum=number/1000;
                break;
            case 2:
                dnum=number%1000/100;
                break;
            case 3:
                dnum=number%1000%100/10;
                break;
            default:
                dnum=number%1000%100%10;
                break;
        }
        PORTB=kc[dnum];
        PORTC=kathode;
        __delay_us(499);
        PORTB=0;
        PORTC=0;
        kathode=kathode*2;
    }
}



void main(void) {
    OSCCON = 0x70; //PLL有効で1倍(0)、8MHz(1110)、無意味な絶対的値(0)、訳もわからず(00)=0b11110000よって8MHz
    //0b0-1110-0-00で1倍8MHzだと思う。そして内部発振使用。
    ANSELA = 0x01; //0b00000001でAN0をアナログ入力
    ANSELB = 0x00; //全部デジタル
    TRISA = 0x01; //入力は(1)
    TRISB = 0x00; //全部出力
    TRISC = 0x00;
    ADCON0 = 0x01; //0b0-00000-0-1
    ADCON1 = 0xa0; //0b1-010-0-0-00
    PORTA = 0;
    PORTB = 0;
    PORTC = 0;
    
    while(1){
        display(1234);
    }
}