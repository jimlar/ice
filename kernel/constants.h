//
// constants.h
// This is where all magic numbers are
//

#ifndef _CONSTANTS_H
#define _CONSTANTS_H

// Misc. constants
#define NULL 0L
#define NIL_PTR 0L

// This should be the same as in the kickstart code
#define PAGE_SIZE 4096

// Amount of stack kernel needs
#define KERNEL_STACK_SIZE PAGE_SIZE>>2

// Max number of processes (should be dynamic later)
#define MAX_PROCESSES 4096

//Protection levels
#define INT_PRIVILEGE      0
#define SYS_PROC_PRIVILEGE 1
#define USER_PRIVILEGE     3

//Process constants
#define NIL_PROC 0L


#define FALSE 0
#define TRUE  1


//Signaling constants, somewhat posix like.. =)
//floating point exception
#define SIGFPE  8
//trace trap
#define SIGTRAP 5
#define SIGBUS  10
#define SIGEMT  7
#define SIGILL  4
//segmentation violation
#define SIGSEGV 11

#endif


