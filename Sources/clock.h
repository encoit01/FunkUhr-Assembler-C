/*  Header for Clock module

    Computerarchitektur / Computer Architecture
    (C) 2020/2021 J. Friedrich, W. Zimmermann 
    Hochschule Esslingen

    Author:   W.Zimmermann, Sept 08, 2020
*/

// Data type for clock events
typedef enum { NOCLOCKEVENT=0, SECONDTICK } CLOCKEVENT;

//Data type for display events
typedef enum { NOUPDATE=0, UPDATEDISPLAY } DISPLAYEVENT;

// Global variable holding the last clock event
extern CLOCKEVENT clockEvent;
extern DISPLAYEVENT displayEvent;

// Public functions, for details see clock.c
void initClock(void);
void processEventsClock(CLOCKEVENT event);
void setClock(int weekday, int day, int month, int year, int hours, int minutes, int seconds);
void displayDateTimeClock(DISPLAYEVENT event);
void timeZone(void);
int daysOverflowed(int month, int day);
void setLeapYear();
