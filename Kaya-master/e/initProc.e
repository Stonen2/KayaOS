#ifndef INITPROC
#define INITPROC

pteOS_t KSegOS; /*OS page table*/
pte_t kuSeg3;   /*Seg3 page table*/
swap_t swapPool[SWAPPOOLSIZE];

int swapSem;
int mutexArr[SEMNUM];
int masterSem;

uProc_t uProcs[MAXUPROC];

extern void uProcInit();

#endif
