/*  Header for LCD module

    Computerarchitektur / Computer Architecture
    (C) 2020/2021 J. Friedrich, W. Zimmermann 
    Hochschule Esslingen

    Author:   W.Zimmermann, Sept 08, 2020
*/

// Public functions, for details see lcd.asm
void initLCD(void);
void writeLine(char* text, unsigned char zeilennummer);
void delay_10ms(void);