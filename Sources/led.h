/*  Header for LED module

    Computerarchitektur / Computer Architecture
    (C) 2020/2021 J. Friedrich, W. Zimmermann 
    Hochschule Esslingen

    Author:   W.Zimmermann, Sept 08, 2020
*/

// Public functions, for details see led.asm
void initLED(void);
void toggleLED(unsigned char mask);
void setLED(unsigned char mask);
void clrLED(unsigned char mask);
