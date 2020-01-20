/*  PHASE 2
    Written by NICK STONE AND SANTIAGO SALAZAR
    Base code and Comments from PROFESSOR MIKEY G
    Finished on 10/30/19
*/

/*********************************************************************************************
                            Module Comment Section
    A SYSCALL exception occurs when a SYSCALL assembler instruction is executed. After
    the processor and ROM-Exception handler actions when a SYSCALL point exception is
    raised, continued executing the nucleus Exception handler. This nreakpoints, are
    distinguidhed from a Breakpoint, exceptions by the contents of Cause.ExcCode in the
    SYSOldArea. There are 8 Sys calls, which are going to be defined with more detail in
    each function documentation.
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

/* Global Variables from initial.e */
extern int processCount;
extern int softBlockCount;
extern pcb_t *currentProcess;
extern pcb_t *readyQue;
extern int semD[SEMNUM];

/* Variables for maintaining CPU time in scheduler.e*/
extern cpu_t TimeSpentComputing;
extern cpu_t Quantumstart;

/*  Declaration of exceptions and helper fucntions. Further documentation will be provided
    in the actual function.*/
HIDDEN void Syscall1(state_t *caller);
HIDDEN void Syscall2();
HIDDEN void Syscall3(state_t *caller);
HIDDEN void Syscall4(state_t *caller);
HIDDEN void Syscall5(state_t *caller);
HIDDEN void Syscall6(state_t *caller);
HIDDEN void Syscall7(state_t *caller);
HIDDEN void Syscall8(state_t *caller);
HIDDEN void PassUpOrDie(state_t *caller, int triggerReason);
void PrgTrapHandler();
void TLBTrapHandler();
void CtrlPlusC(state_t *oldState, state_t *newState);
HIDDEN void TimeToDie(pcb_t *harambe);

/*  There are 8 System calls (Syscall 1 through Syscall 8) that our Handler must look out
    for these first 8 System calls the Kernel Mode must be active in order for these commands
    to execute. If this is not the case, then the appropiate program trap would be execute. 
    Parameters: None
    Return: Void*/
void SYSCALLHandler()
{
    /*Create variables to be used throughout this function*/
    state_t *prevState;
    state_t *program;
    pcb_PTR newprocess;

    unsigned int prevStatus;
    int sysReason;
    int mode;

    int * sema;
    int *sem;

    /*Assign a State * to be the SYS CALL OLD areaa*/
    prevState = (state_t *)SYSCALLOLDAREA; /* prevState status*/
    
    /*Assign the status*/
    prevStatus = prevState->s_status;
    
    /*Assign which sys call is being called*/
    sysReason = prevState->s_a0;
    
    /*Uses the compliment to determine the mode I'm in*/
    mode = (prevStatus & UMOFF); 
    
    /*Sys call less than 1 or greater than 9 pass up or die they are not built to be handled */
    if ((sysReason < SYSCALL1) || (sysReason > SYSCALL8))
    {
        /*Passup or die the previous state and specify a sys trap*/
        PassUpOrDie(prevState,SYSTRAP);
    }

    if(mode != ALLOFF){/*If the sys call is calling a 1-8 and is in user mode*/
        program = (state_t *)PRGMTRAPOLDAREA;
        unsigned int temp;
    
        /*Copy the old state to the program trap old area to call program trap handler*/
        CtrlPlusC(prevState, program);
        
        /*setting Cause.ExcCode in the Program Trap Old Area to Reserved Instruction */
        temp = (program->s_cause)& ~(0xFF);
        program->s_cause = (temp |(10 << 2));

        /*Program Trap Handler */
        PrgTrapHandler();

    }
    /* increment prevState's PC to next instruction */
    prevState->s_pc = prevState->s_pc + FOUR;

    /*Switch statement to determine which Syscall we are about to do. If there is no case, we
        execute the default case. The following switch statement utilizes function calls on 
        sys calls that are complex and are better seperated by calling multiple functions. The
        sys calls that are listed in the switch statement are several lines long and are easy 
        to follow the functions that are not in the switch statement are SYS 1,2, 5, 8 with the
        others being listed in the sys call. */
    switch (sysReason)
    {
    /*Create process (1)*/
    case SYSCALL1:
        Syscall1(prevState);
        break;
    /*Terminate Process (2)*/
    case SYSCALL2:
        Syscall2();
        break;
    /*Verhogen Process (3)*/
    /*  When this service is requested, it is interpreted by the nucleus to request to perform a Verhogen
    (V) operation on a sempahore. This is requested by placing 3 in a0, abd Verhogened in a1.
    Parameter:  state* caller
    Return: Void
    */
    case SYSCALL3:

        /*Create a new process block and set it to NULL*/
        newprocess = NULL;

        /*Cast the semaphore value in a1 to an int start and set it to a variable*/
        int * sema = (int *) prevState ->s_a1;
        
        /* increment semaphore*/
        (*sema) = (*sema) + ONE;
        
        if (*sema <= ZERO)
        { /* waiting in the semaphore */

            /*Set the new process to a blocked process to the corresponding semaphore*/
            newprocess = removeBlocked(sema);
            
            /*If its not null*/
            if (newprocess != NULL)
            { /*add it to the ready queue */
                insertProcQ(&readyQue, newprocess);
            }
        }
        LDST(prevState); /* returns control to caller */
        break;

    /*Passeren Process (4)*/
    /*  When this service is requested, it is interpreted by the nucleus to request to perform a Passeren
    (P) operation on a sempahore. This is requested by placing 4 in a0, and Passerened in a1.
    Parameter:  state* caller
    Return: Void
    */
    case SYSCALL4:

        /*Same process cast the semahore value from a1 and set it to a variable*/
        sema = (int *)prevState->s_a1; /* decrement semaphore */
        /*Decrement that bitch */
        (*sema) = (*sema) - ONE;
        if (*sema < ZERO)
        { /* there is something controlling the semaphore */
            /*Copy the state then insert onto the blocked and increment the softblock count and call scheduler*/
            CtrlPlusC(prevState, &(currentProcess->p_s));
            insertBlocked(sema, currentProcess);
            scheduler();
        }
        /* nothing had control of the sem, return control to caller */
        LDST(prevState);
        break;

    /*Specify the Exception State Vector (5)*/
    case SYSCALL5:
        Syscall5(prevState);
        break;
    
    /*Get CPU Time Process (6)*/
    /*Syscall6:  "Get_CPU_Time"
    This service is in charge of making sure that the amount of time spent being processed is tracked by
    each Process Block that is running.
        Parameters: State_t * caller
        Return: Void*/
    case SYSCALL6:
    
    /*Copy the state of the caller*/
    CtrlPlusC(prevState, &(currentProcess->p_s));
    
    /*Get the updated time then add the difference to the time spent processing*/
    STCK(TimeSpentComputing);
    
    /*Track the amout of time spent processing and add this to the previous amount of process time*/
    currentProcess->p_timeProc = currentProcess->p_timeProc + (TimeSpentComputing - Quantumstart);
    
    /*Store the new updated time spent processing into the v0 register of the process state*/
    currentProcess->p_s.s_v0 = currentProcess->p_timeProc;
    
    /*Load the Current Processes State*/
    LDST(&(currentProcess ->p_s));
    break;
    
    /*Wait for clock Process (7)*/
    /*  Syscall 7 performs a syscall 4 on the Semaphore associated to clock timer
        Knowing that this clock also has a syscall 3 performing on it every 100 milliseconds
        Parameters: State_t* Caller
        Return: Void*/
    case SYSCALL7:
        sem = (int *)&(semD[SEMNUM - ONE]);
        (*sem) = (*sem) - 1;
        if (*sem < ZERO)
        {/*Sem is less than 0 block the current process*/
            
            /*Copy the state increment it onto the blocked list and increment softblock count*/
            CtrlPlusC(prevState, &(currentProcess->p_s));
            
            insertBlocked(sem, currentProcess);
            /*Increment that we have another process soft block so that it does not starve*/
            softBlockCount = softBlockCount + ONE;
        }
        /*Process is soft blocked call to run another process*/
        scheduler();
        break;

    /*Wait for IO Device Process (8)*/
    case SYSCALL8:

        Syscall8(prevState);
        break;
    }

}

/**************************  SYSCALL 1 THROUGH 8 FUNCTIONS    ******************************/

/*  This service creates a new process (progeny) of the caller process. a1 contains the physical
    address of a processor state area at the rime instruction is executed. The processsor state
    is uded as the initial state of the newly birth child.
    Parameter:  state* caller
    Return:     -0 in V0 if the process was done effectively
                -1 in V0 if the process was NOT done because of lack of resources.*/

 void Syscall1(state_t *caller)
{
    /*make a new process and allocate a process block to it */
    pcb_t *birthedProc = allocPcb();
    /*Clean the new process block just in case*/
   /* birthedProc = clean(birthedProc);*/
    /*If the new process is null then We know there is no way to allocate a process*/
    if (birthedProc != NULL)
    {
        /*Makes the new process a child of the currently running process calling the sys call */
        insertChild(currentProcess, birthedProc);
        /* Inserts the new process into the Ready Queue*/
        insertProcQ(&readyQue, birthedProc);
        /*Copy the state into the prorcess state of the new process that we have allocated*/
        CtrlPlusC((state_t *)caller->s_a1, &(birthedProc->p_s));
        /*WE were able to allocate thus we put 0 in the v0 register SUCCESS!*/
        caller->s_v0 = ZERO;
        /*Increment process count */
        processCount = processCount + ONE;
    }
    else
    {
        /*We did not have any more processses able to be made so we send back a -1*/
        /*FAILURE*/
        caller->s_v0 = -ONE;
    }
    /*Load the state of the state that called the sys 1*/
    LDST(caller);
}

/*  This services causes the executing process to be anihilated along with all its children, grand
    children and so on. Execution of this instruction does not complete until everyone has been
    exterminated
    Parameters: None
    Return: Void*/
 void Syscall2()
{
    /*Isolate the process being terminated from its dad and brothers*/
    if(currentProcess == NULL){
        /*No Current Process then we panic*/
        PANIC();
    }
    /*Send the Current Process to the helper function*/
    TimeToDie(currentProcess);
    /*call scheduler*/
    scheduler();
}

/*  When this service is requested, it will save the contentes of a2 and a3 and pass them to handle the
    respective exceptions (TLB, PGMTRAP, SYS) while this process is executing. Each process may request
    a SYS5 only ONCE for each of the exceptions types, more than one call will trigger SYS2 and Nuke the
    process (error occured).
    Parameter:  state* caller
    Return: Void
    */
 void Syscall5(state_t *caller)
{
     if(currentProcess == NULL){
        /*No Current Process then we panic*/
        PANIC();
    }
    int trapCause;
    trapCause = (int) caller->s_a1;

    if (trapCause == ZERO)
    { /*TLB TRAP*/
        if (currentProcess->p_newTLB != NULL)
        { /* already called sys5 */
            Syscall2();
        }
        /* assign exception values */
        currentProcess->p_oldTLB = (state_t *)caller->s_a2;
        currentProcess->p_newTLB = (state_t *)caller->s_a3;
    }
    else if (trapCause == ONE)
    { /*Program Trap*/
        if ((currentProcess->p_newProgramTrap) != NULL)
        { /* already called sys5 */
            Syscall2();
        }
        /* assign exception values */
        currentProcess->p_oldProgramTrap = (state_t *)caller->s_a2;
        currentProcess->p_newProgramTrap = (state_t *)caller->s_a3;
    }
    else if (trapCause == TWO)
    {

        if ((currentProcess->p_newSys) != NULL)
        { /* already called sys5 */
            Syscall2();
        }
        /* assign exception values */
        currentProcess->p_oldSys = (state_t *)caller->s_a2;
        currentProcess->p_newSys = (state_t *)caller->s_a3;
    }else{
        Syscall2();
    }
    LDST(caller);
}

/*  This service perofroms a Syscall 5 operation on the semaphore that the nucles maintains for the IO
    device by the values in a1, a2, and a3 (optionally). The nucleus performs a V operation on the
    semaphore whenever that device generates an interrupt.
    Return:     Device Status in v0 (Once the process resumes after the occurrence of the anticipated
                interrupt)*/
 void Syscall8(state_t *caller)
{
    /*
     if(currentProcess == NULL){*/
        /*No Current Process then we panic*/
        /*PANIC();
    }*/
    int lineNo;     /*  line number*/
    int dnum;       /*  device number*/
    int termRead;   /*  Read/Write Terminal*/
    int index;
    int *sem;
    
    /* what device is going to be computed*/
    lineNo = (int)caller->s_a1;
    dnum = (int)caller->s_a2;
    termRead = (int)caller->s_a3; 

    /*Find the index of the proper semaphore */
    index = lineNo -DEVWOSEM + termRead;
    index = index * DEVPERINT;
    index = index + dnum;

    sem = &(semD[index]);
   
    /*Decrement the semaphore*/
   (*sem) = *sem -ONE;
    if (*sem < ZERO)
    {
        /*Copy state put it on the blcok queue increment softblock and call scheduler*/
        CtrlPlusC(caller, &(currentProcess->p_s));
        insertBlocked(sem, currentProcess);
        softBlockCount = softBlockCount + ONE;
        scheduler();
    }
}
/**************************  HANDLERS FUNCTIONS    ******************************/

/*If an exception has been encountered, it passes the error to the appropiate handler, if no exception
    is found, it Nukes the procees till it pukes.
    Parameters: state_t *caller
    Return: Void
    */
void PassUpOrDie(state_t *caller, int triggerReason)
{
    switch (triggerReason)
    {
    case TLBTRAP: /*0 is TLB EXCEPTIONS!*/
        if ((currentProcess->p_newTLB) == NULL)
        {
            Syscall2();
        }
        else
        {
            /*Copy the caller to the old and load the new*/
            CtrlPlusC(caller,currentProcess ->p_oldTLB);
            LDST((currentProcess ->p_newTLB));
        }
        break;

    case PROGTRAP: /*1 is Program Trap Exceptions*/

        if ((currentProcess->p_newProgramTrap) == NULL)
        {
            Syscall2();
        }
        else
        {
            /*Copy the caller to the old and load the new*/
            CtrlPlusC(caller,currentProcess ->p_oldProgramTrap);
            LDST((currentProcess ->p_newProgramTrap));
        }
        break;

    case SYSTRAP: /*2 is SYS Exception!*/
        if ((currentProcess->p_newSys) == NULL)
        {
            Syscall2();
        }
        else
        {
            /*Copy the caller into the old and load the new */
            CtrlPlusC(caller,currentProcess ->p_oldSys);
            LDST((currentProcess ->p_newSys));
        }
        break;
    }
}


/**************************  HELPER FUNCTIONS    ******************************/

/*Recursively removes all the children of the head, and starts hunting them down one by one.
    It kills them if they are in the ASL, ReadyQueue or currentProcess. Adjust the process count
    as the process are being terminated.
    Parameters: pcb_t * HeadPtr
    Return: Void
    */

HIDDEN void TimeToDie(pcb_t * harambe)
{
    /*Look through until we no longer have a child*/
    while(!emptyChild(harambe)){
        /*Recursive call with the first child*/
        TimeToDie(removeChild(harambe));
    }

    /*If the semaphore is NULL it is not blocked*/
    if(harambe ->p_semAdd == NULL){
        /*Remove it from the Ready Queue */
        outProcQ(&readyQue, harambe);
    }

    /*If the current Process is equal to the parameter Process*/
    if(harambe == currentProcess){
        /*Remove the child from the parents child list*/
        pcb_t * test;
        outChild(harambe);
    }

    else
    {
        /*We know the process is blocked*/
        int * tracksem = harambe ->p_semAdd;
        /*Remove it from the blocked list*/
        outBlocked(harambe);
        if (tracksem >= &(semD[ZERO]) && tracksem <= &(semD[SEMNUM]))
            {
                /*Decrement the softblock */
                softBlockCount = softBlockCount - ONE;
            }
            else
            {
                /*Increment the Semaphore*/
                *tracksem = *tracksem + ONE;
            }
    }

    /*Free the process block then decrement the process count */
    freePcb(harambe);
    processCount = processCount - ONE;
}

/*Gets triggered when the executing process performs an illegal operation. Therefore, since  this is
    triggered when a PgmTrap exception is raised, execution continues with the nucleus’s PgmTrap exception
    handler. The cause of the PgmTrap exception will be set in Cause.ExcCode in the PgmTrap Old Area.
    Parameters: None
    Return: Void
     */
void PrgTrapHandler()
{
    /*Set a State to be the program trap old area specify a program trap then call pass up or die */
    PassUpOrDie((state_t *)PRGMTRAPOLDAREA, PROGTRAP);
}

/*Gets triggered when μMPS2 fails in an attempt to translate a virtual address into its corresponding
    physical address. Therefore, since  this is triggered when a TLB exception is raised, execution
    continues with the nucleus’s TLB exception handler. The cause of the TLB exception will be set in
     Cause.ExcCode in the TLB Old Area.
     Parameters: None
    Return: Void
     */
void TLBTrapHandler()
{
    /*Set State to the TLBMGOLDAREA and specify a TLB Trap then call pass up or die*/
    PassUpOrDie((state_t *)TLBMGMTOLDAREA, TLBTRAP);
}

/*  This state will copy all of the contents of the old state into the new state
    Parameters: State_t * oldstate, State_t* NewState
    Return: Void*/
extern void CtrlPlusC(state_t *oldState, state_t *newState)
{
    /*Loop through all of the registers in the old state and write them into the new state*/
    int i;
    for (i = 0; i < STATEREGNUM; i++)
    {
        newState->s_reg[i] = oldState->s_reg[i];
    }
    /*Move all of the contents from the old state into the new*/
    newState->s_asid = oldState->s_asid;
    newState->s_status = oldState->s_status;
    newState->s_pc = oldState->s_pc;
    newState->s_cause = oldState->s_cause;
}

pcb_PTR clean(pcb_PTR temp){
    /*Just in case re clean the process block*/
    temp->p_child = NULL;
    temp->p_nextSib = NULL;
    temp->p_prevSib = NULL;
    temp->p_prnt = NULL;
    temp->p_next = NULL;
    temp->p_prev = NULL;
    temp ->p_semAdd = ZERO;
    temp ->p_timeProc = ZERO;
    temp ->p_s.s_asid =ZERO;
    temp -> p_s.s_cause = ZERO;
    temp ->p_s.s_pc = ZERO;
     int i;
    for (i = 0; i < STATEREGNUM; i++)
    {
       temp->p_s.s_reg[i] = ZERO;
    }
    temp ->p_s.s_status = ZERO;
    temp->p_oldSys = NULL;
    temp->p_newSys = NULL;
    temp->p_oldTLB = NULL;
    temp->p_newTLB = NULL;
    temp->p_oldProgramTrap = NULL;
    temp->p_newProgramTrap = NULL;


    return temp;
}
