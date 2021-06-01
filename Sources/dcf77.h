/*  Header for DCF77 module

    Computerarchitektur / Computer Architecture
    (C) 2020/2021 J. Friedrich, W. Zimmermann 
    Hochschule Esslingen

    Author:   W.Zimmermann, Sept 08, 2020
*/


// Data type for DCF77 signal events
typedef enum { NODCF77EVENT=0, VALIDZERO, VALIDONE, VALIDSECOND, VALIDMINUTE, INVALID } DCF77EVENT;

// Global variable holding the last DCF77 event
extern DCF77EVENT dcf77Event;

// Public functions, for details see dcf77.c
void initDCF77(void);
DCF77EVENT sampleSignalDCF77(int currentTime);
void processEventsDCF77(DCF77EVENT event);

// Prototypes of functions simulation DCF77 signals, when testing without
// a DCF77 radio signal receiver
void initializePortSim(void);                   // Use instead of initializePort() for simulator testing
char readPortSim(void);                         // Use instead of readPort() for simulator testing
void decodeDateTime();
int checkParity(int, int);
void setLeapYear(int);