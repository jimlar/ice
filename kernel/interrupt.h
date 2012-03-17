//
// interrupt.h
// This is the interrupt handling class for my O/S project
// 
// (c) Jimmy Larsson 1998
//

#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include "types.h"

//How many irq's do we have?
#define NO_OF_IRQ_VECTORS 16

// Which vector is IRQ0
#define IRQ0_VECTOR   0x28
// Which vector is IRQ8, (right after IRQ0-7)
#define IRQ8_VECTOR   0x30

//Exception numbers
#define DIVIDE_INT              0  //divide error
#define DEBUG_INT	        1  //single step
#define NMI_INT			2  //non-maskable interrupt
#define BREAKPOINT_INT	        3  //software breakpoint
#define OVERFLOW_INT		4  //overflow
#define BOUNDS_CHECK_INT        5  //bounds check fail
#define INVALID_OPCODE_INT	6  //invalid opcode
#define COPROC_NOT_AVAIL_INT	7  //coprocessor not availiable
#define DOUBLE_FAULT_INT        8  //double fault
#define COPROC_SEG_OVERRUN_INT  9  //coprocessor segment overrun
#define INVALID_TSS_INT		10 //invalid TSS
#define SEGMENT_NOT_PRESENT_INT 11 //segment not present
#define STACK_FAULT_INT		12 //stack exception
#define GENERAL_PROTECTION_INT	13 //general protection fault
#define PAGE_FAULT_INT		14 //page fault
#define COPROC_ERROR_INT	16 //coprocessor error

#define SYS_CALL_INT            32 //System call's trap vector (everything lower than 40 ok.) 

class InterruptHandler
{
 private:
  static void init_i8259 (void); // initialize PC INT controllers
  static int default_irq_handler (int irq); //Spurious IRQ

  static int irq_usemap;
  static int unlocking_allowed;

 public:
  
  static int initialize (void); 
  static void lock (void);      //turn off interrupts (cli)
  static void unlock (void);    //turn on interrupts  (sti)
  static int register_handler (irq_handler_t handler, int irq);
  static void enable_irq (int irq);
  static void disable_irq (int irq);

  static void unhold (void);  //Flush held up interrupts
  static void exception (unsigned ex_nr); //Exception handler
};

#endif













