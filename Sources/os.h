/*  Header for Operating System module

    Computerarchitektur / Computer Architecture
    (C) 2020/2021 J. Friedrich, W. Zimmermann 
    Hochschule Esslingen

    Author:   W.Zimmermann, Sept 08, 2020
*/

#define OSNUMTASKS 8	 		// Number of operating system tasks

typedef struct 				// Data type for tasks
{   void (*osTaskFunction)(enum event);	// Function pointer to task
    int *osPEvent;			// Event, which trigger task execution
} osTCB;

void initOS(osTCB osTaskList[]);	// Function to start the operating system, does never return