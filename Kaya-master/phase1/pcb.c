/*  PHASE 1
    Written by NICK STONE AND SANTIAGO SALAZAR
    Base code and Comments from PROFESSOR MIKEY G
    Finished on 9/5/2019
*/

/*********************************************************************************************
                            Module Comment Section
This Modules is the Queues Manager of the Operating System. The data Structure is a double linked
list with a tail pointer. Furthermore each process has an associated tree of child processes which
the Manager maintains. The child process Data structure is a double linked linear list that is
Null terminated. Pcb_t is located in Types.h
**********************************************************************************************/
#include "../h/const.h"
#include "../h/types.h"
#include "../e/pcb.e"


/*Globally defines pcbList_h.*/
pcb_PTR pcbList_h;


/*  Insert the element pointed to by p into the pcbFree list
    Parameters: pcb_t *p
    Return: void*/
void freePcb(pcb_PTR p){
    /*
    Set the Node we are addings Next pointer to the current head
    Set head to be the new node
    */
    p->p_next = pcbList_h;
    pcbList_h = p;
}

/*  Removes an element from the pcbFree list, provide initial values for ALL of
    the ProcBlk’s fields (i.e. NULL and/or 0)
    Parameters:
    Return: -Null (if the pcbFree list is empty)
            -Pointer to the removed element (if the pcbFree list is NOT EMPTY)*/

pcb_PTR allocPcb(){
   /*   If the Head Node is NULL then the list is empty and we return NULL*/

    pcb_PTR temp;
    /*If the Free List has no room to Allocate Return NULL*/
    if(pcbList_h == NULL){
        return NULL;
    }
    /* We then make a temporary Pointer to the head of the free list
    Then we move the head pointer in the free list to be the next process in the list
    Then we set all of the temporary pointers values to NULL or 0
    Return Temp
    */
    temp = pcbList_h;
    pcbList_h = pcbList_h-> p_next;
    /*Setting the Fields of the New PCB*/
    temp->p_child = NULL;
    temp->p_nextSib = NULL;
    temp->p_prevSib = NULL;
    temp->p_prnt = NULL;
    temp->p_next = NULL;
    temp->p_prev = NULL;


    temp->p_semAdd = NULL;
    temp ->p_timeProc = 0; /* no CPU time */


    /*Syscall5 exceptions pointes are going to be defined*/
    temp->p_oldSys = NULL;
    temp->p_newSys = NULL;
    temp->p_oldTLB = NULL;
    temp->p_newTLB = NULL;
    temp->p_oldProgramTrap = NULL;
    temp->p_newProgramTrap = NULL;

    /*Return this Node */
    return temp;
}

/*  Initialize the pcbFree list to contain all the elements of the static array of
    MAXPROC ProcBlk’s.*/

void initPcbs(){
    /*
    Set the pcbFree_h to be NULL
    Then We set the pcbFree_h to be the first item in the array
    Then we loop through the array knowing that it is static so we know the size of the array
    Then We create a temp pointer (Pcb_t * temp) to be pcbFree_h -> p_next
    Keep setting the p_next value to be the p_next element in the array
    No Return Value
    We are done
    */
    static pcb_t PcbInitialization[MAXPROC];
    /*Set the Free List Value to be NULL*/
    pcbList_h = NULL;

    int i;
    i=0;
    /*Create 20 Proc blocks and Put them on the Free List*/
    for(i = 0; i < MAXPROC; i++){
        freePcb(&PcbInitialization[i]);
    }





}

/* This method is used to initialize a variable to be tail pointer to a
    process queue.
    Return a pointer to the tail of an empty process queue; i.e. NULL.

 */
pcb_PTR mkEmptyProcQ(){
    /*Make an Empty Queue... Null */
    return NULL;
}

/*  Checks wether the tail of the queue is empty or not.
    Paramater: pcb_t
    Return  TRUE (if the queue whose tail is pointed to by tp is empty.)
            FALSE (if the queue whose tail is pointed to by tp is NOT empty.)*/
int emptyProcQ(pcb_PTR tp){
    /*It the Tail pointer is Empty Return True if Not Return False*/
    return (tp == NULL);
}

/*  Inserts the ProcBlk pointed to by p into the process queue
    Parameters: **tp, *p
    Return: Void */
void insertProcQ(pcb_PTR *tp, pcb_PTR p){

/*The Tail Pointer pointed too is Empty */
    if(emptyProcQ(*tp)){                 /*Case 1: There is no node.*/

        /*Take the Proc Block that is being passed in and Since it is the only node set the values
        * To be itself
        */
        p->p_next = p;
        p->p_prev = p;
    }
    else if((*tp)->p_next == (*tp)){      /*Case 2: There is only one node*/
    /*If there is only One Node then the following Logic is executed
    *THe New Nodes Next is assigned to be the tail Pointer as well as its previous
    * Then the Tails New Previous is the new Node and the Next is also the new node
    * Being added
    */
        p->p_next = (*tp);
        p->p_prev = (*tp);
        (*tp)->p_prev = p;
        (*tp)->p_next = p;
    }else{                              /*Case 3: There is more than one node.*/
    /* We set a New Node to be the Head node of the list
    *Then we set the Tail pointers Next pointer to be the new node
    * THen we set the new nodes next pointer to be the head
    * Then we set the Heads previous pointer to be the new node
    * Then we set the new Nodes previous to point to the old tail
    * update the tail pointer
    */
        pcb_PTR tempHead = (*tp) -> p_next;     /*Initialize the Queue head*/
        (*tp) -> p_next = p;                  /*Adds the new node*/
        p -> p_next = tempHead;             /*Fixes Pointers*/
        tempHead -> p_prev = p;
        p -> p_prev = (*tp);
        (*tp) = p;
    }
    /*New Tail Is updated to be the New Node*/
    (*tp) = p;
}

/*  Remove the first (i.e. head) element from the process queue
    Parameters: **tp
    Return: NULL    (if the process queue is empty)
            pcb_t   (if the process queue is NOT empty)*/
pcb_PTR removeProcQ(pcb_PTR *tp){
    /*If the List is Empty then we return NULL*/
    if(emptyProcQ((*tp))){                    /*Case 1: Queue is empty*/
        return NULL;
    }
    /**/
    else if((*tp)->p_next == (*tp)){            /*Case 2: Queue has 1 nodes*/
        pcb_t *temp = *tp;
        *tp = mkEmptyProcQ();
        return temp;
    }
    /*We make a temp to point to the head Node
    *We then set the tails pointer to point to the head nodes next
    * we then set the heads next nodes previous pointer to point to the tail
    * Then we return the head
    */
    else{                                  /*Case 3: Queue has 2 or more nodes*/
        pcb_PTR temp = (*tp)-> p_next;
        (*tp) -> p_next = temp-> p_next;
        temp -> p_next -> p_prev = (*tp);
        return temp;
    }
}


pcb_PTR outProcQ(pcb_PTR *tp, pcb_PTR p){

    /*
    If the Tail pointer points to NUll then the list is empty
    Error Condition and should return NULL
    */
    if(emptyProcQ(*tp)){
        return NULL;
    }
    /*
    if the Tail pointer is the pcb_t that is being removed
    Then we must rewrite and reassign the tail pointer
    as well as rearrange the data structure
    Tail pointer now equals the previous Node
    and that that the head Node now points the prev to the tail pointer
    */
    else if((*tp)==p){

        if((*tp)->p_next == *tp){
            (*tp) = mkEmptyProcQ();
        }else{
            (*tp) -> p_prev -> p_next = (*tp) -> p_next;
            (*tp) -> p_next -> p_prev = (*tp) -> p_prev;
            *tp = (*tp) -> p_prev;
        }

        return p;
    }
    /*
    If it is not the tail pointer then we must loop through the structure to find the P pointer
    Simply start with the tails next and contiue looping until we are back at the tail pointer
    if the new node is equal to P thus they both point to Pcb_t
    then we rearrange the data structure and then return temp
    if Not found return NULL
    */
    else{
        pcb_PTR temp;
        temp = headProcQ(*tp);

        while(temp !=*tp && temp!= p){

            temp = temp ->p_next;
        }

        if(temp == *tp){

            return NULL;
        }

        p -> p_prev -> p_next = p ->p_next;
        p ->p_next ->p_prev = p ->p_prev;
         return temp;;


    }
}


/*  Checks wether the process queue has more than one node.
    Parameters: pcb-t * tp
    Return: NULL     (if the process queue is empty)
            tp->p_next (if the process queue is NOT empty). */
pcb_PTR headProcQ(pcb_PTR tp){
    /*If the list is empty there is no Head so return NULL
    *Else Return the Tail pointers next (Head)
    */
    if(emptyProcQ(tp)){
        return NULL;
    }
    return tp->p_next;
}


/************************* Process Tree Maintenance *************************************/



/*  Checks wether the Blok queue has Children or not.
    Parameters: pcb-t * p
    Return: True        (if the proBlk has no Children)
            False       (if the proBlk has no Children). */
int emptyChild(pcb_PTR p){

    return (p->p_child== NULL);
}


/*  Make the ProcBlk pointed to by p a child of the ProcBlk pointed to by prnt
    Parameters: pcb-t * p
    Return: void*/
void insertChild(pcb_PTR p_prnt, pcb_PTR p){
    if(emptyChild(p_prnt)){               /*There is no children*/
        p_prnt -> p_child= p;
        p -> p_prnt = p_prnt;
        p -> p_prevSib = NULL;
        p -> p_nextSib = NULL;
    }
    else{                              /*There is 1 or more children*/
        p -> p_prnt = p_prnt;
        p_prnt -> p_child-> p_prevSib = p;
        p -> p_nextSib = p_prnt -> p_child;
        p -> p_prevSib = NULL;
        p_prnt ->  p_child= p;
    }
}


/*  Make the first child of the ProcBlk pointed to by p no longer a
    child of p.
    Parameters: pcb_t
    Return: Null        (if there is no children)
            pcb_t * p   (to the removed child of ProcBlk)*/
pcb_PTR removeChild(pcb_PTR p){
    pcb_PTR temp;
    if(emptyChild(p)){                             /*No Children*/
        return NULL;
    }else if(p -> p_child-> p_nextSib == NULL){       /*One Child*/
        temp = p -> p_child;
        temp -> p_prnt = NULL;
        temp -> p_nextSib = NULL;
        temp -> p_prevSib = NULL;
        p -> p_child= NULL;

        return temp;                                /*More than one children*/
    }else{
        temp = p->p_child;
        temp -> p_nextSib -> p_prevSib = NULL;
        p-> p_child= temp -> p_nextSib;
        temp -> p_nextSib = NULL;
        temp -> p_prnt = NULL;

        return temp;
    }
}

/* Make the ProcBlk pointed to by p no longer the child of its parent.
    If the ProcBlk pointed to by p has no parent, return NULL; otherwise,
    return p. Note that the element pointed to by p need not be the first
    child of its parent. */
pcb_PTR outChild(pcb_PTR p){
    if(p == NULL){
        return NULL;
    }
    if(p ->p_prnt == NULL){
        return NULL;
    }

    /*Only CHild*/

    if(p->p_prevSib == NULL && p->p_nextSib == NULL && p == p->p_prnt ->p_child){
        p->p_prnt ->p_child = NULL;
        p-> p_prnt = NULL;
        return p;
    }
    /*first child*/

    if(p == p->p_prnt ->p_child){
        p -> p_prnt ->p_child = p->p_nextSib;
        /*Referencing a NULL pointer*/
        p->p_nextSib ->p_prevSib = NULL;
        p->p_prnt = NULL;
        p->p_nextSib = NULL;
        return p;
    }
    /*Last child */
   if(p->p_nextSib == NULL){
        p->p_prevSib ->p_nextSib = NULL;
        p->p_prevSib = NULL;
        p->p_prnt = NULL;
        return p;
    }
      /*
    middle child
    */
    if(p->p_prevSib != NULL && p->p_nextSib != NULL){
        p->p_prevSib ->p_nextSib = p->p_nextSib;
        p->p_nextSib ->p_prevSib = p->p_prevSib;
        p->p_nextSib = NULL;
        p-> p_prevSib = NULL;
        p->p_prnt = NULL;
        return p;
    }





    return NULL;

}
