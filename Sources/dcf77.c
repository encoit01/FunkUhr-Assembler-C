/*  Radio signal clock - DCF77 Module

    Computerarchitektur / Computer Architecture
    (C) 2020/2021 J. Friedrich, W. Zimmermann 
    Hochschule Esslingen

    Author:         W.Zimmermann, Sept 08, 2020
    Modified by:    Enes Coskunyürek, 764552
                    Tolgahan Kandemir, 761469
*/

/*
; N O T E:  T H I S  M O D U L E  I S  N O T  F U L L Y  F U N C T I O N A L
;
; The file contains function prototypes for functions you have to implement.   
; See the sections marked as "--- Add your code here --- --- ??? ---".
;
*/


#include <hidef.h>                                      // Common defines
#include <mc9s12dp256.h>                                // CPU specific defines
#include <stdio.h>

#include "dcf77.h"
#include "led.h"
#include "clock.h"
#include "lcd.h"

/* ********** GLOBAL VARIABLES **********
 * dcf77Event:      Global variable to holf the last DCF77 event
 * tLowCounter:     Counter to validate a low period.
 * minutecounter:   Counter to validate the minute period
 * secondCounter:   Counter to validate a second perriod
 * position:        Referrer to index the bit-sequence
 * invalid:         Variable to show a invalid bit-sequence
 * bits[]:          Array to store the BCD bit-sequence     
*/
DCF77EVENT dcf77Event = NODCF77EVENT;
int tLowCounter = 0;
int minuteCounter = 0;
int secondCounter = 0;
int position = 0;
int invalid = 0;
int bits[59];



/* ********** EXTERN GLOBAL VARIABLES **********
 * zone:            Extern global variable to determine DE or US timezone 
 * maxDayOfMonths:  Extern global variable to store maximum days of a month
*/
extern zone;
extern int maxDayOfMonths[];

/* ********** MODULE VARIABLES **********
 * minutes:         variable to store minutes of bit-sequence   
 * hours:           variable to store hours of bit-sequence
 * day:             variable to store day of bit-sequence
 * month:           variable to store month of bit-sequence
 * year:            variable to store year of bit-sequence
 * weekDecoder:     variable to store weekDecoder of bit-sequence
 * lastSignal:      variable to store the lastSignal, to determine rising or falling edges
*/
static int minutes;
static int hours;
static int day;
static int month;
static int year;
static int weekDecoder;
static char lastSignal = 1;

static int  dcf77Year=2020, dcf77Month=3, dcf77Day=1, dcf77Hour=2, dcf77Minute=0, dcf77Second=0, dcf77Weekday=0; //dcf77 Date and time as integer values


// ****************************************************************************
//  Initialize DCF77 module
//  Called once before using the module
void initDCF77(void) {   
    setClock(dcf77Weekday, dcf77Day, dcf77Month, dcf77Year, dcf77Hour, dcf77Minute, dcf77Second);

    #ifdef SIMULATOR
        initializePortSim();
    #else
        initializePort();
    #endif    
}

// ****************************************************************************
//  Read and evaluate DCF77 signal and detect events
//  Must be called by user every 10ms
//  Parameter:  Current CPU time base in milliseconds
//  Returns:    DCF77 event, i.e. second pulse, 0 or 1 data bit or minute marker


/* ********** Function sampleSiglanDCF77(...) ********** 
 * Description:     Read and evaluate DCF77 signal and detect events.
 *                  Must be called by user every 10ms
 * Parameter:       Current CPU time base in milliseconds
 *                  NOTE: currentTime is not used
 * Return:          DCF77EVENT - represents the actual event
 */
DCF77EVENT sampleSignalDCF77(int currentTime) {
    DCF77EVENT event = NODCF77EVENT;
    char currentSignal;

    #ifdef SIMULATOR
        currentSignal = readPortSim();			// Sample simulated DCF77 signal
    #else
        currentSignal = readPort();				// Sample DCF77 signal
    #endif   

    // CHECK IF CURRENTSIGNAL HAS CHANGED WITH LAST SIGNAL - EDGE DETECTED
    if(currentSignal != lastSignal) {
        
        // ~RISING EDGE
        if(currentSignal > 0) {
            
            // ASSUME THAT EVENT IS INVALID AND CHECK OPPOSITE 
            event = INVALID;

            // CLEAR LED ON PORT B.1 
            clrLED(0x02);

            // CHECK FOR VALID ONE
            if(tLowCounter >= 170 && tLowCounter <= 230) {
                event = VALIDONE;
            }
            
            //CHECK FOR VALID ZERO
            if(tLowCounter >= 70 && tLowCounter <= 130) {
                event = VALIDZERO;
            }

            // RESET LOWCOUNTER
            tLowCounter = 0;

        // ~FALLING EDGE
        } else {
            // RESET LOWCOUNTER
            tLowCounter = 0;

            // ASSUME THAT EVENT IS INVALID AND CHECK OPPOSITE 
            event = INVALID;

            // TURN LED ON PORT B.1 ON
            setLED(0x02);

            // CHECK FOR VALID MINUTE
            if(minuteCounter >= 1900 && minuteCounter <= 2100) {
                event = VALIDMINUTE;
            }

            // CHECK FOR SECOND
            if(secondCounter >= 900 && secondCounter <= 1100) {
                event = VALIDSECOND;
                if (PTH & 0x04){
                    //Button3 pressed
                    timeZone();
                }
                secondCounter = 0;
            }
            
            // RESET COUNTER
            minuteCounter = 0;
            secondCounter = 0;
        }
    } else {
        // ~NO EDGE
        event = NODCF77EVENT;

        // CHECK FOR INVALID MINUTE COUNTER
        if(minuteCounter >= 2100) {
            event = INVALID;
        }
    }

    // INCREMENT COUNTERS
    minuteCounter += 10;
    tLowCounter += 10;
    secondCounter += 10;

    // UPDATE LAST SIGNAL
    lastSignal = currentSignal;

    return event;
}


/* ********** FUNCTION: processEventxDCF77 **********
 * Description:     Function that reads the triggered events.
 * Parameters:      DCF77EVENT event
 * return:          -
 */
void processEventsDCF77(DCF77EVENT event) {   
    
    switch(event){

        // CASE INVALID: 
        case INVALID: 
            // set invalid to show, that the signal was invalid
            invalid = 1; 

            // reset reading position 
            position = 0;

            // clear LED on PORT B.2
            clrLED(0x04);
            break;

        // CASE VALIDZERO:
        case VALIDZERO: 
            // read a valid zero into the bits array
            if(invalid == 0) bits[position] = 0; 
            break;

        // CASE VALIDSECONDS
        case VALIDSECOND: 
            // increment position referrer every valid second 
            if(invalid == 0) position++; 
            break;

        // CASE VALIDMINUTE:
        case VALIDMINUTE: 
            // check if position referrer is at the end -> decodeDateTime
            if(position == 58) decodeDateTime();    

            // reset position referrer and invalid
            position = 0;
            invalid = 0; 
            break;    

        // CASE VALIDONE:
        case VALIDONE: 
            // read a valid 1 into the bits array
            if(invalid == 0) bits[position] = 1; 
            break;

        // CASE NODCF77EVENT:
        case NODCF77EVENT: 
            break;
    }
}


/* ********** FUNCTION: devodeDateTime() **********
 * Description:     Function to decode the DCF77 bit-sequence.
 *                  Check parity bits for a valid sequence and 
 *                  synchronize the day and time. 
 * Parameter:       -
 * Return:          - 
 */
void decodeDateTime() {
    minutes = 0;
    hours = 0;
    day = 0;
    month = 0;
    year = 0;
    weekDecoder = 0;

    // READ MINUTES: 7 BITS
    minutes += bits[21];
    minutes += bits[22] * 2;
    minutes += bits[23] * 4;
    minutes += bits[24] * 8;
    minutes += (bits[25] * 10);
    minutes += (bits[26] * 20);
    minutes += (bits[27] * 40);
    
    // READ HOURS: 7 BITS
    hours += bits[29];
    hours += bits[30] * 2;
    hours += bits[31] * 4;
    hours += bits[32] * 8;
    hours += (bits[33] * 10);
    hours += (bits[34] * 20);

    // READ DAYS: 6 BITS
    day += bits[36];
    day += bits[37] * 2;
    day += bits[38] * 4;
    day += bits[39] * 8;
    day += (bits[40] * 10);
    day += (bits[41] * 20);

    // READ MONTHS: 5 BITS
    month += bits[45];
    month += bits[46] * 2;
    month += bits[47] * 4;
    month += bits[48] * 8;
    month += (bits[49] * 10);

    // READ YEARS: 8 BITS
    year += bits[50];
    year += bits[51] * 2;
    year += bits[52] * 4;
    year += bits[53] * 8;
    year += (bits[54] * 10);
    year += (bits[55] * 20);
    year += (bits[56] * 40);
    year += (bits[57] * 80);
    year += 2000;
    
    // READ WEEKDAY: 3 BITS
    weekDecoder += bits[42];
    weekDecoder += bits[43] * 2;
    weekDecoder += bits[44] * 4;

    // CHECK PARITY
    // CASE: INVALID PARITY
    if(checkParity(21, 27) || checkParity(29, 34) || checkParity(36, 57)) {
        invalid = 1;
        clrLED(0x04);
    
    // CASE: VALID PARITY
    } else {
        setLeapYear(year);
        setLED(0x04);
    }

    // SET CLOCK
    if(invalid != 1) {
        // CASE: USA TIMEZONE
        if(zone == 1) { 
            setClock(weekDecoder, day, month, year, hours - 6, minutes, 0);
        
        // CASE: DE TIMEZONE
        } else {        
            setClock(weekDecoder, day, month, year, hours, minutes, 0);
        }
    }
}

/* ********** FUNCTION: checkParity(...) ********** 
 * Description: Function to check for event parity.
 * Parameters:  int startIndex, int endindex
 * Returns:     0 -> VALID PARITY
 *              1 -> INVALID PARITY
 */
int checkParity(int startIndex, int endIndex) {
    // COUNTER TO COUNT ONES IN BIT-SEQUENCE
    int counter = 0;

    // LOOP FROM STARTINDEX TO ENDINDEX 
    for(startIndex; startIndex <= (endIndex + 1); startIndex++){
        if(bits[startIndex] == 1) counter++; //increment counter 
    }

    // CHECK VALID PARITY AND RETURN 0 OR 1
    if(counter % 2 != 0) return 1;
    return 0;
}
