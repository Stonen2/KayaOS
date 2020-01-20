/*  PHASE 2
    Written by NICK STONE AND SANTIAGO SALAZAR
    Base code and Comments from PROFESSOR MIKEY G
    Finished on 10/30/19
*/

/*********************************************************************************************
                            Module Comment Section
This module processes all types of device interrupts, inculding Interval Timer interrupts,
and converting device interrupts into V operations on the appropriate semaphores.
    
DECIDED TO CALL SCHEDULER instead of giving back time to the process that was interrupted
Keeps the overall flow of the program and since there is no starvation, eventually the
process will get its turn to play with the processor.
**********************************************************************************************/
#include "../h/const.h"
#include "../h/types.h"

#include "../e/asl.e"
#include "../e/pcb.e"

#include "../e/initial.e"
#include "../e/interrupts.e"
#include "../e/exceptions.e"
#include "../e/scheduler.e"

#include "/usr/local/include/umps2/umps/libumps.e"


/* Global Variables from initial.e*/
extern int processCount;
extern int softBlockCount;
extern pcb_t *currentProcess;
extern pcb_t *readyQue;
extern int semD[SEMNUM];

/*We want to use the copy state fucntion from exceptions*/
extern void CtrlPlusC(state_PTR oldstate, state_PTR NewState);

/*2 additional functions to help compute the device number and call the scheduler*/
int finddevice(int linenumber);
void CallScheduler();


/*Function that willacknowledge the highest priority interrupt and then give control
    over to the scheduler. This should not be called as an independent function, if
    so, it will cause a PANIC().*/
void IOTrapHandler()
{
    /*Line number bit map that is in the cause register of the state stored in interrupt old area*/
    unsigned int offendingLine;

    /*Need to Determine Device Address and the Device semaphore number*/
    int templinenum;
    int lineNumber;
    int devsemnum;
    int devicenumber;
    
    /*V operation sempahore variables */
    int * semad;
    int* semaphoreAddress;
    
    /*Store the device status to place in v0*/
    int deviceStatus;
    
    /*Another timing variable*/
    pcb_t *blockProc;
    state_PTR caller;
    
    /*Get the state of the offending interrupt*/
    caller = (state_t *)INTERRUPTOLDAREA;
    
    /*Shift 8 since we only care about bits 8-15*/
    offendingLine = caller ->s_cause >> EIGHT;
    
    if ((offendingLine & MULTICORE) != ZERO)
    { /*Mutli Core is on */
        PANIC();
    }
    else if ((offendingLine & CLOCK1) != ZERO)
    {
        /*The process has spent its quantum. Its time to start a new process .*/
        CallScheduler();
        /*Clock 1 Has an Interrupt */
    }

    else if ((offendingLine & CLOCK2) != ZERO)
    {
        /*Access the Last clock which is the psuedo clock*/
        semaphoreAddress = (int *) &(semD[SEMNUM-ONE]);

       /*Free all of the processes that are currently blocked and put them onto the ready queue*/
        while(headBlocked(semaphoreAddress) != NULL)
        {
            /*Remove from the blocked list*/
           blockProc = removeBlocked(semaphoreAddress);
            /*if not null then we put that bitch back onto the ready queue*/
            if(blockProc != NULL){
                insertProcQ(&readyQue,blockProc);
                /*One less softblock process */
                softBlockCount--;
            }
        }
         /*Set the semaphore back to 0*/
        *semaphoreAddress = ZERO;
        /*Load the clock with 100 Milliseconds*/
        LDIT(PSUEDOCLOCKTIME);
        CallScheduler();
    }
    else if ((offendingLine & DISKDEVICE) != ZERO)
    {
        /*Disk Device is on  */
        lineNumber = DI;
    }
    else if ((offendingLine & TAPEDEVICE) != ZERO)
    {
        /*Tape Device is on */
        lineNumber = TI;
    }
    else if ((offendingLine & NETWORKDEVICE) != ZERO)
    {
        /*Network Device is on */
        lineNumber = NETWORKI;
    }
    else if ((offendingLine & PRINTERDEVICE) != ZERO)
    {
        /*Printer Device is on */
        lineNumber = PRINTERI;
    }
    else if ((offendingLine & TERMINALDEVICE) != ZERO)
    {
        /*Terminal Device is on */
        lineNumber = TERMINALI;
    }
    /*Not recognized so go ahead and panic for me*/
    else
    {
        PANIC();
    }

    /*Call the helper function since we have the line number and need to find the device number*/
    devicenumber = finddevice(lineNumber);
    /*Offest the Line number*/
    templinenum = lineNumber - DEVWOSEM;
    /* 8 devices per line number*/
    devsemnum = templinenum * DEVPERINT;
    /*We know which device it is */
    devsemnum = devsemnum + devicenumber;
    device_t * OffendingDeviceRegister;
    
    /*Numbers got from Umps2 Principles of Operation book. Chapter 5, page 32 (NO MAGIC Numbers)*/
    OffendingDeviceRegister = (device_t *)(0x10000050 + (templinenum * 0x80) + (devicenumber * 0x10));
    
    if (lineNumber == TERMINT)
    {
        /*Terminal*/
        if ((OffendingDeviceRegister->t_transm_status & 0xF) != READY)
        {
                /*Set the device status*/
                deviceStatus = OffendingDeviceRegister->t_transm_status;
                /*Acknowledge*/
                OffendingDeviceRegister->t_transm_command = ACK;
        }
        else
        {
            /*Semaphore number + 8 */
            devsemnum = devsemnum + DEVPERINT;
            /*Save the status*/
            deviceStatus = OffendingDeviceRegister->t_recv_status;
            /*Acknowledge*/
            OffendingDeviceRegister->t_recv_command = ACK;
            /*fix the semaphore number for terminal readers sub device */
        }
    }
    /*Not a terminal pretty straight forward*/
    else
    {
        /*Non terminal Interrupt*/
        deviceStatus = OffendingDeviceRegister->d_status;
        /*Acknowledge the interrupt*/
        OffendingDeviceRegister->d_command = ACK;
    }
    /*Get the semaphore for the device causing the interrupt*/
    semad =&(semD[devsemnum]);
    /*Increment by 1 */
    (*semad)= (*semad) +ONE;
    if ((*semad) <= ZERO)
    {   /*Remove one from the blocked list and if that is not null*/
       blockProc = removeBlocked(semad);
        if (blockProc != NULL)
        {
            /*Set the status in the v0 register decrement the softblock count and insert it onto the ready queue*/
            blockProc-> p_s.s_v0 = deviceStatus;
            softBlockCount = softBlockCount - ONE;
            insertProcQ(&readyQue,blockProc);
        }
    }
    CallScheduler();
    /*Interrupt has been Handled!*/
}

/*----------------------------------------HELPER FUNCTIONS---------------------------------------------*/

/**
 * Take in the line number of the interrupt that is offending. We will then bit shift until we find the first device causing
 * The interrupt
 * parameter: Int Line number of interrupt
 * Return type: Int of the offending device number
*/
int finddevice(int linenumber)
{
    /*Set some local variables*/
    int i;
    /*area that is causing the interrupt that has the map of the device*/
    devregarea_t * tOffendingDevice;
    
    /*ProperLineNumber is to track the line number - 3 */
    int ProperLineNumber;
    
    /*The bit map of the device bit map and a bit map only containning the first bit*/
    unsigned int LineBitmap;
    unsigned int  BitMapActive;
    
    /*Device number*/
    int offendingdevicenumber;
    
    /*We know that the line number - 3 DEVNOSEM*/
    ProperLineNumber = linenumber -DEVWOSEM;
    /*Set this to be the RAMBASEADDR*/
    tOffendingDevice = (devregarea_t *) RAMBASEADDR;
    
    /*make a copy of the bit map */
    LineBitmap = tOffendingDevice->interrupt_dev[ProperLineNumber];
    
    /*Only care about the first bit */
    BitMapActive = FIRSTBIT;
    
    /*8 Total devices to look through */
    for (i = 0; i < TOTALDEVICES; i++)
    {
        /*Bit wise and if the value is not 0 Device is interrupting */
        if ((LineBitmap & BitMapActive) == BitMapActive)
        {
            /*Device number is equal to the index and we are done looping */
            offendingdevicenumber = i;
            break;
        }
        /* shift the map to the right 1 to check the next device */
        LineBitmap = LineBitmap >> ONE;
    }
    /*Return the device number*/
    return offendingdevicenumber;
}

/*Function in charge of either putting the current process back on the ready queue and calling the scheduler
* Or just going to call the scheduler
* Parameters: None
* Return: void
*/
void CallScheduler()
{
    /*set a temp state to point to the interrupt old area*/
    state_t *temp;
    temp =  (state_t *)INTERRUPTOLDAREA;

    /*If the current process is null we need to put it back onto the ready queue*/
    if (currentProcess != NULL)
    {
         /*if the process is still around need to copy its contents over*/
        CtrlPlusC(temp, &(currentProcess->p_s));
        insertProcQ(&readyQue, currentProcess);
    }
    /*No Current Process go ahead and call the scheduler for the next process*/
      scheduler();
    
}
