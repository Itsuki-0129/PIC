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
#pragma config FOSC = INTOSC    // Oscillator Selection (INTOSC oscillator: I/O function on CLKIN pin)
#pragma config WDTE = OFF       // Watchdog Timer Enable (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable (PWRT disabled)
#pragma config MCLRE = OFF      // MCLR Pin Function Select (MCLR/VPP pin function is digital input)
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
#pragma config LVP = OFF        // Low-Voltage Programming Enable (High-voltage on MCLR/VPP must be used for programming)

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

//割り込み関数のプロトタイプ宣言
void __interrupt() isr(void);

//EEPROM初期データ定義
//アドレス0はタイマー設定値の初期値、残りはダミーデータ
__EEPROM_DATA (0x05, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);

//グローバル変数
unsigned char timerValue; //タイマー設定値(main関数の最初でEEPROMから設定値を読み出し、この変数に入れておく)
unsigned char timerCount; //現在のタイマー残り時間(main関数ではこの値を減らしながらタイマーカウント。割り込み処理関数では呼ばれたらtimerValueに戻す)



void main(void) {
    //0b0-1110-0-00で1倍8MHzだと思う。そして内部発振使用。
    OSCCON = 0x70; //PLL有効で1倍(0)、8MHz(1110)、無意味な絶対的値(0)、訳もわからず(00)=0b11110000よって8MHz
    //アナログ入力設定
    ANSELA = 0x01; //0b00000001でAN0をアナログ入力
    ANSELB = 0x00; //全部デジタル
    //入出力ピン設定
    TRISA = 0x01; //入力はRA0だけ
    TRISB = 0x80; //入力はRB7だけ
    TRISC = 0x00;
    //ADコンバータ設定
    ADCON0 = 0x01; //0b0-00000-0-1
    ADCON1 = 0xa0; //0b1-010-0-0-00
    PORTA = 0;
    PORTB = 0;
    PORTC = 0;
    
    //変数宣言
    unsigned short timer;       //時間計測
    unsigned short duty = 1;    //PWMのデューティーサイクル(0~100)
    unsigned short i;           //for文で使う変数
    long long result = 0;           //AD変換後の値
    
    //タイマー時間変数
    //EEPROMに保存してある値を使用する
    //timerValueは1バイトで宣言済み
    timerValue = eeprom_read(0);
    
    
    //PWM機能
    CCPTMRS1 = 0b00;
    //データシートのP230, 23.5: CCP制御レジスタを参照
    CCP5CON = 0b00001100;
    
    //周期とデューティーサイクルの設定
    //データシートのP205, 22.5: Timer2/4/6 制御レジスタを参照
    //プリスケーラ値は16
    T2CONbits.T2CKPS = 0b10;
    
    //周期を1msに設定するので、1ms=(PR2+1)*4*(1/8000000)*16より124が導かれる。
    PR2 = 124;
    //デューティーサイクルを計10ビットの値として、上位8ビットをCCPR5Lレジスタに、下位2ビットをCCP5CONレジスタのDC5Bビットに、書き込む
    //上位8ビットについて、2進数では2で割ると1桁右にシフトできるため、2*2=4で割る。
    CCPR5L = (5*duty)/4;
    //下位2ビットについて、そのままでも上位桁が無視されてOKだが、一応ANDで下位２ビットだけをマスクする。
    CCP5CONbits.DC5B0 = 5*duty&0b11;
    //PWMをスタートさせる。これでPWM信号が生成され始める。
    T2CONbits.TMR2ON = 1;
    
    while(1){
        result = adconv();
        result = result*100/1024;
        CCPR5L = 5*result/4;
        CCP5CONbits.DC5B0 = 5*result&0b11;
        display(result);
        
        //スイッチが押されたらタイマースタートする
        //タイマースタート前にタイマー時間設定ボタンが押されたらその処理を行う
        while (RB7) {
            //ADコンバータ読み取り開始
            GO = 1;
            
            //読み取り完了待ち
            while(GO);
            
            //タイマー時間を減らす
            timerValue = 999;
            
            //タイマー設定値をEEPROMに書き込む
            eeprom_write(0, timerValue);
            
            //RB7割り込み設定
            IOCIE = 1;
            IOCBN7 = 1;
            GIE = 1;
            
            //タイマー開始
            timerCount = timerValue;
            do {
                timerCount--;
                display(timerCount);
            } while ( timerCount );
            
            //割り込み禁止
            GIE = 0;
            IOCIE = 0;
            IOCBN7 = 0;
            
            
        }
    }
}

//AD変換関数
int adconv() {
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO == 1);
    return ADRES;
}


//割り込み関数
void __interrupt() isr(void) {
    //設定時間を表現するために現在の設定時間を7セグに表示する
    for (int i=0; i<1000; i++) {
        display(timerValue);
    }
    
    //timerを最初から開始する
    timerCount = timerValue;
    
    //割り込みフラグをクリアする
    IOCBF7 = 0;
}