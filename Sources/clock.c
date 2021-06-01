/*  Radio signal clock - Free running clock

    Computerarchitektur / Computer Architecture
    (C) 2020/2021 J. Friedrich, W. Zimmermann 
    Hochschule Esslingen

    Author:         W.Zimmermann, Sept 08, 2020
    Modified by:    Enes Coskunyürek, 764552
                    Tolgahan Kandemir, 761469
*/

#include <stdio.h>

#include "clock.h"
#include "lcd.h"
#include "led.h"
#include "dcf77.h"

// Defines
#define ONESEC  (1000/10)                       // 10ms ticks per second
#define MSEC200 (200/10)


// Global variable holding the last clock event
CLOCKEVENT clockEvent = NOCLOCKEVENT;
DISPLAYEVENT displayEvent = NOUPDATE;

/* ********** MODULE GLOBAL VARIABLES **********
 * days, months:    days represent the actual year of the clock.
 *                  The maximum value depends on the maximum days of  month -> [1;28-31]
 *                  months represent the actual month of the year -> [1-12]
 *  
 * year:            years represent the actual year of the clock, starting with 2000        
 *  
 * hrs, mins, secs: Representation of the clock Time e.g 14:45:28
 *          
 * ticks:           Counter variable for ticker
 */
static char days  = 0, months = 0;
static int  years = 0;
static char hrs = 0, mins = 0, secs = 0;
static int uptime = 0;
static int ticks = 0;


/* ********** GLOBAL VARIABLES **********
 * weekdays:        Pointer to reference days of the week.  
 * 
 * descZone:        Pointer to reference the Timezone of the clock.
 *                  Used for printing 
 *
 * zone:            Global Variable to differentiate in which zone the clock is set to.
 *
 * weekDecoder:     Used to map weekdays into String representing weekdays
 *
 * maxDayOfMonths:  Array to differenciate the maximum days of a month
 */
char *weekdays;
char *descZone;
int zone = 0;
int weekDecoder = 0;
int maxDayOfMonths[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };


// ****************************************************************************
//  Initialize clock module
//  Called once before using the module
void initClock(void) {
    displayEvent = UPDATEDISPLAY;
}

// ****************************************************************************
// This function is called periodically every 10ms by the ticker interrupt.
// Keep processing short in this function, run time must not exceed 10ms!
// Callback function, never called by user directly.
void tick10ms(void) {
    if (++ticks >= ONESEC)                      // Check if one second has elapsed
    {   clockEvent = SECONDTICK;                // ... if yes, set clock event
        ticks=0;
        setLED(0x01);                           // ... and turn on LED on port B.0 for 200msec
    } else if (ticks == MSEC200)
    {   clrLED(0x01);
    }
    uptime = uptime + 10;                       // Update CPU time base

    dcf77Event = sampleSignalDCF77(uptime);     // Sample the DCF77 signal
}

// ****************************************************************************
// Process the clock events
// This function is called every second and will update the internal time values.
// Parameter:   clock event, normally SECONDTICK
// Returns:     -


/* ********** Function: processEventsClock(CLOCKEVENT event) **********
 * Description: Processing of the clock events.
 *              This function is called every seconds an will update the interal time values.
 * Parameters:  CLOCKEVENT event        prevent the clock from being incremented at NOCLOCKEVENT            
 * Return:      -
 */
void processEventsClock(CLOCKEVENT event) {
    // CASE: NOCLOCKEVENT -> return
    if (event==NOCLOCKEVENT) {
        return;
    }
    
    // INCREMENT SECONDS & HANDLE SECONDS OVERFLOW
    secs++;
    if(secs >= 60) {
        secs = 0;

        //INCREMENT MINUTES & HANDLE MINUTES OVERFLOW
        mins++;
        if(mins >= 60) {
            mins = 0;

            //INCREMENT HOURS & HANDLER HOURS OVERFLOW
            hrs++;
            if(hrs >= 24) {
                hrs = 0;

                //INCREMENT DAYS & HANDLE DAYS OVERFLOW
                days++;
                setLeapYear(years); 
                if(daysOverflowed(months - 1, days) == 1) {
                    days = 1;

                    // INCREMENT MONTHS & HANDLE MONTHS OVERFLOW
                    months++;
                    if(months >= 13) {
                        months = 1;

                        // INCREMENT YEARS 
                        years++;
                    }
                }
            }
        }
    }

    displayEvent = UPDATEDISPLAY;
}

// ****************************************************************************
// Allow other modules, e.g. DCF77, so set the time
// Parameters:  day, month, year, hours, minutes, seconds as integers
// Returns:     -

/* ********** FUNCTION: setClock(...) **********
 * Description: Function to reset the time of the clock and the date.
 *              Differnciate between US zone and DE zone
 * Parameters:  weekday, day, month, year, hours, minutes
 * Return:      -
 */
void setClock(int weekday, int day, int month, int year, int hours, int minutes, int seconds) { 
    // HANDLE LEAP YEAR
    setLeapYear(years);

    // FIT DATE AND TIME INTO US ZONE -> zone == 1
    if(zone == 1){

        // HANDLE UNDERFLOW OF HOURS
        if(hours < 0){
            hours = 24 + hours;  

            // HANDLE UNDERFLOW OF WEEKDAY 
            if(weekday == 1){
                weekday = 7;
            }else{
                weekday--;
            }

            // HANDLE UNDERFLOW OF DATE 
            day = day - 1;
            if(day == 0){
                month--;

                // HANDLER UNDERLOW OF MONTH
                if(month == 0){
                    month = 12;
                    year = year - 1;
                }

                // MAXIMUM DAY OF ACTUAL MONTH
                day = maxDayOfMonths[month - 1];
            }
        }
    
    // FIT DATE AND TIME INTO DE ZONE -> zone == 0
    } else if (zone == 0) {

        // HANDLE OVERFLOW OF HOURS
        if(hours >= 24) {
            hours = hours - 24;

            // HANDLE OVERFLOW OF WEEKDAY
            if(weekday == 7) {
                weekday = 0; 
            } else {
                weekday++;
            }
            
            // HANDLE OVERFLOW OF DAYS
            day = day + 1;   
            if(daysOverflowed(month - 1, day) == 1) {
                day = 1;

                // HANDLE OVERFLOW OF MONTH
                month = month + 1;
                if(month >= 13) {
                    month = 1;
                    year++;
                }
            }
        }
    }

    // MAP WEEKDAY
    weekDecoder = weekday;
    if(weekday == 0) weekdays = "---: ";
    if(weekday == 1) weekdays = "Mon: ";
    if(weekday == 2) weekdays = "Tue: ";
    if(weekday == 3) weekdays = "Wed: ";
    if(weekday == 4) weekdays = "Thu: ";
    if(weekday == 5) weekdays = "Fri. ";
    if(weekday == 6) weekdays = "Sat: ";
    if(weekday == 7) weekdays = "Sun: ";

    // SET DATE AND TIME 
    days     = (char) day;
    months   = (char) month;
    years    = (int)  year;
    hrs      = (char) hours;
    mins     = (char) minutes;
    secs     = (char) seconds;
    ticks    = 0;
}

// ****************************************************************************
// Display the time derived from the clock module on the LCD display, line 0
// Parameter:   -
// Returns:     -

/* ********** FUNCTION: displayDateTimeclock(...) **********
 * Description: Display the time derived from the clock module on the LCD display, line0;
 *              Display the date and weekday derived from the clock module on LCD display, line1;
 * Parameter:   DISPLAYEVENT event
 * Returns:     
 */
void displayDateTimeClock(DISPLAYEVENT event) {
    char uhrzeit[32];
    char datum[32];
    
    if (event==NOUPDATE) return;

    // DEFINE ZONES FOR PRINTING
    if(zone == 1){
        descZone = "US";
    }else{
        descZone = "DE";
    }
    
    (void) sprintf(uhrzeit, "%02d:%02d:%02d  %s", hrs, mins, secs, descZone, zone);
    writeLine(uhrzeit, 0);

    (void) sprintf(datum, "%s%02d.%02d.%04d", weekdays, days, months, years);
    writeLine(datum, 1);
}

/* ********** FUNCTION: timezone() **********
 * Description:     Function to switch the timeZone from US into DE and vice versa
 * Parameter:       -
 * Return:          -
*/
void timeZone() {
    // EU MODE -> US MODE
    if(zone == 0){
        zone = 1;
        setClock(weekDecoder, days, months, years, (hrs-6), mins, secs);
    
    // US MODE -> EU MODE
    }else if(zone == 1){
        zone = 0;
        setClock(weekDecoder, days, months, years, (hrs+6), mins, secs);
    } 
}

/* ********** FUNCTION: daysOverflowed(...) **********
 * Description:     Function to specify if the days have been overflowed
 * Parameter:       int month, int day
 * Return:          0 -> FALSE, 1-> TRUE
*/
int daysOverflowed(int month, int day) {
    // CONFIGURE CASE OF LEAP YEAR
    if(years % 400 == 0 || (years % 4 == 0 && years % 100 != 0)) {
        maxDayOfMonths[1] = 29;
    }

    // CHECK FOR OVERFLOWED DAYS AND RETURN 1 IF OVERFLOWED
    if( day > maxDayOfMonths[month]) {
        return 1;
    }

    // NO OVERFLOW
    return 0;
}

/* ********** FUNCTION: setLeapYear(...) **********
 * Description:     Function to set the leap year, if neccessary
 * Parameter:       int year
 * Return:          -
*/
void setLeapYear(int year) {
    // CASE: LEAP YEAR
    if(year % 400 == 0 || (year % 4 == 0 && year % 100 != 0) ) {
        maxDayOfMonths[1] = 29;
    
    // CASE: NO LEAP YEAR
    } else {
        maxDayOfMonths[1] = 28;
    }
}