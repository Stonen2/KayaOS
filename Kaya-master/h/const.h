#ifndef CONSTS
#define CONSTS

/**************************************************************************** 
 *
 * This header file contains utility constants & macro definitions.
 * 
 ****************************************************************************/

/* Hardware & software constants */
#define PAGESIZE 4096 /* page size in bytes */
#define WORDLEN 4     /* word size in bytes */
#define PTEMAGICNO 0x2A

#define ROMPAGESTART 0x20000000 /* ROM Reserved Page */

/* timer, timescale, TOD-LO and other bus regs */
#define RAMBASEADDR 0x10000000
#define TODLOADDR 0x1000001C
#define INTERVALTMR 0x10000020
#define TIMESCALEADDR 0x10000024

/* utility constants */
#define TRUE 1
#define FALSE 0
#define ON 1
#define OFF 0
#define HIDDEN static
#define EOS '\0'

#define NULL ((void *)0xFFFFFFFF)

/* vectors number and type */
#define VECTSNUM 4

#define TLBTRAP 0
#define PROGTRAP 1
#define SYSTRAP 2

#define TRAPTYPES 3

/* device interrupts */
#define DISKINT 3
#define TAPEINT 4
#define NETWINT 5
#define PRNTINT 6
#define TERMINT 7

#define DEVREGLEN 4   /* device register field length in bytes & regs per dev */
#define DEVREGSIZE 16 /* device register size in bytes */

/* device register field number for non-terminal devices */
#define STATUS 0
#define COMMAND 1
#define DATA0 2
#define DATA1 3

/* device register field number for terminal devices */
#define RECVSTATUS 0
#define RECVCOMMAND 1
#define TRANSTATUS 2
#define TRANCOMMAND 3

/* device common STATUS codes */
#define UNINSTALLED 0
#define READY 1
#define BUSY 3

/* device common COMMAND codes */
#define RESET 0
#define ACK 1

/* operations */
#define MIN(A, B) ((A) < (B) ? A : B)
#define MAX(A, B) ((A) < (B) ? B : A)
#define ALIGNED(A) (((unsigned)A & 0x3) == 0)

/* Useful operations */
#define STCK(T) ((T) = ((*((cpu_t *)TODLOADDR)) / (*((cpu_t *)TIMESCALEADDR))))
#define LDIT(T) ((*((cpu_t *)INTERVALTMR)) = (T) * (*((cpu_t *)TIMESCALEADDR)))

#define MAXPROC 20
#define MAXINT 0xFFFFFFFF
#define QUANTUM 5000
#define IOCLOCK 100000


/*  Explanation of the SEMNUM number. (49)
        There are 8 terminals (x2 (each terminal its schizophrenic))
        There are 8 tapes
        There are 8 printers
        There are 8 Networks
        There are 8 disks
        1 timer
        ARR LAYOUT: DISKS, TAPES, NETWORKS, PRINTERS, TERMINALS READ, TERMINALS WRITE, PSEUDOCLOCK 
        */
#define SEMNUM 49
#define DEVWOSEM 3  /*Device without Semaphore. First 3 devices doesnt have semaphores (disks, tapes, nerworks)*/

/* NEW Processor State Areas */
    /* SYSCALL BREAK*/
#define SYSCALLNEWAREA 0x200003D4
#define SYSCALLOLDAREA 0X20000348
    /* PROGRAM TRAP */
#define PRGMTRAPNEWAREA 0x200002BC
#define PRGMTRAPOLDAREA 0x20000230
    /* TLB MANAGEMENT */
#define TLBMGMTNEWAREA 0x200001A4
#define TLBMGMTOLDAREA 0x20000118
    /* INTERRUPTS */
#define INTERRUPTNEWAREA 0x2000008C
#define INTERRUPTOLDAREA 0x20000000

/* ON/OFF bit manipulation*/
#define ALLOFF  0x00000000
#define VMOFF   0xF8FFFFFF      /* Virtual Memory OFF*/
#define VMON2    0x02000000      /* Virtual Memory ON*/
#define VMON1    0x01000000      /* Virtual Memory ON*/
#define UMOFF   0x00000008       /* User Mode OFF (Kernel Mode ON)*/
#define UMON    0xFFFFFFF7     /* User Mode ON (Kernel Mode OFF)*/
#define IMOFF   0xFFFF00FF      /* Interrupt UNMasked */
#define IMON    0x0000FF00      /* Interrupt Masked */
#define IEON	0x00000004      /* Turn interrupts ON*/
#define IECON	0x00000001      /* Turn interrupts current ON */



#define TEON			0x08000000


/*Define Sys Call Exceptions*/

#define SYSCALL1 1 
#define SYSCALL2 2 
#define SYSCALL3 3 
#define SYSCALL4 4 
#define SYSCALL5 5 
#define SYSCALL6 6 
#define SYSCALL7 7 
#define SYSCALL8 8 


/* Define Interrupts  */ 
#define MULTICORE 0x00000001
#define CLOCK1 0x00000002
#define CLOCK2 0x00000004
#define DISKDEVICE 0x00000008
#define TAPEDEVICE 0x00000010
#define NETWORKDEVICE 0x00000020
#define PRINTERDEVICE 0x00000040
#define TERMINALDEVICE 0x00000080


#define C1I 1 
#define C2I 2 
#define DI 3 
#define TI 4 
#define NETWORKI 5 
#define PRINTERI 6 
#define TERMINALI 7 

#define ZERO 0 
#define FIRSTBIT 0x00000001
#define DEVPHYS 0x10000000
#define PSUEDOCLOCKTIME 100000 
#define TOTALDEVICES 8 
#define TEBITON 0x08000000



#define ONE 1
#define TWO 2
#define FOUR 4
#define EIGHT 8


/*Acknowledge a interrupt*/
#define ACK 1 


/*____________PHASE 3___________________*/
#define KUSEGSIZE 32
#define KSEGSIZE 64
#define SWAPPOOLSIZE 16
#define MAXUPROC 1  /*  PHASE 3 Num of processes*/

#define DIRTY   0x00000400
#define VALID	0x00000200
#define GLOBAL	0x00000100

#define UPROCSETUP 0x800000B0;
#define WELLKNOWNSTARTPROCESS 0x800000B0;

#define INTDEVREG 0x10000050
#define DISKREADBLK 3
#define EOT			0
#define EOF			1

#define GETASID 0x00000FC0
#define INTCAUSEMASK 0x7C
#define TLBLOAD 2
#define TLBSTORE 3

#define GET_SEG 0xC0000000
#define SHIFT_SEG 30
#define GET_VPN 0x3FFFF000

#define ROMSTADDY 0x20000000	
/*Just give the OS 50 pages? Not sure how much memory the OS needs tbh*/
#define OSEND ROMSTADDY +  (PAGESIZE * 30 /*I think the number of pages goes here!*/)
/*OS end + 8 devs x Page */
#define TAPESTARTADDY OSEND + (8 * PAGESIZE)
/*Tape buff + 8 devs x Page*/
#define DISKSTARTADDY TAPESTARTADDY + (8 * PAGESIZE)
/*Everyone gets 2 pages of memory to play with */
#define BASESTACKALLOC ((2 * MAXUPROC ) *  PAGESIZE)

/*Begin to allocate some memory starting at this location */
#define ALLOCATEHERE DISKSTARTADDY + BASESTACKALLOC
/*Disk Device Constants*/


#define DISKRESET 0 /*Reset the device and move the boom to cylinder 0*/
#define DISKACK 1   /*ACknowlege a pending INterrupt*/
#define DISKSEEKCYL 2   /*Seek to the specified CYLNUM*/
#define DISKREADBLK 3   /*Read the block located at (HEADNUM,SECTNM) in the current Cylinder and copy it into RAM start at the address in DATA0*/
#define DISKWRITEBLK 4  /*Copy the 4kb of RAM starting at address in Data0 into the block located at (HEADNUM, SECTNUM) in the current cylinder*/


/*Tape Device Constants*/
#define TAPEEOT 0 /*End of Tape*/
#define TAPEEOF 1 /*End of File*/
#define TAPEEOB 2 /*ENd of Block*/
#define TAPETS 3 /*Tape Start*/

#define SYSCALL9 9 
#define SYSCALL10 10 
#define SYSCALL11 11 
#define SYSCALL12 12 
#define SYSCALL13 13 
#define SYSCALL14 14 
#define SYSCALL15 15 
#define SYSCALL16 16 
#define SYSCALL17 17 
#define SYSCALL18 18 



#define SEG3 0xC0000000

#define SIX 6 
#define TWELVE 12
#define ENTRYHIGH 0x20000
#define HEADERMASK 0x2A
#define HEADERSHIFT 24
#define LASTENTRYSEG 0xBFFFF
#define PAGETABLEPROCESS 0x80000
#define ENTRYHIGHPGTABLE 0xC0000
#define ASIDMASK 0x00000FC0




#endif
