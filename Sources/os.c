/*  Operating System (OS) module

    Computerarchitektur / Computer Architecture
    (C) 2020/2021 J. Friedrich, W. Zimmermann 
    Hochschule Esslingen

    Author:   W.Zimmermann, Sept 08, 2020
*/

#include "os.h"

void initOS(osTCB osTaskList[])
{   int i;

//  Operating system scheduling loop
    for(;;)					
    {   for (i=0; i<OSNUMTASKS; i++)		// Loop through all tasks in task list
        {   
            if (osTaskList[i].osPEvent && *osTaskList[i].osPEvent)        // -- Call task, if event was triggered
            {   if (osTaskList[i].osTaskFunction)
                {   osTaskList[i].osTaskFunction(*osTaskList[i].osPEvent);
                }
                *osTaskList[i].osPEvent = 0;	// -- Reset event		         
            }
        }
    }
}
