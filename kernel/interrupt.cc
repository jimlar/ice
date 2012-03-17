//
// interrupt.cc
// This is the interrupt handling class for my O/S project
// 
// (c) Jimmy Larsson 1998
//

#include "types.h"
#include "constants.h"
#include "console.h"
#include "scheduler.h"
#include "io_ports.h"
#include "descriptor.h"
#include "interrupt.h"

//This is a trick to let assembler code access these var/functs.
//Really ugly!

unsigned char InterruptHandler_k_reenter = 0;
irq_handler_t InterruptHandler_irq_table[NO_OF_IRQ_VECTORS];
p_elem_t     *InterruptHandler_held_int_head = NIL_PROC;
p_elem_t     *InterruptHandler_held_int_tail = NIL_PROC;

//Flush held interrupts
void (*InterruptHandler_unhold)(void) = InterruptHandler::unhold;
//Exception handler
void (*InterruptHandler_exception)(unsigned int) = InterruptHandler::exception;


//
// Local constants used, no need to export them to *.h file...
//
// IO Ports

#define INT1_CTL       0x20
#define INT2_CTL       0xa0
#define INT1_CTLMASK   0x21
#define INT2_CTLMASK   0xa1

// The cascade interrupt number
#define CASCADE_IRQ   2



// Initialization Command Words, these are different for AT and PS/2.
// (some are different for master/slave controllers too)
// They are used to init the PIC hardware.

// ICW4 needed, cascade, 8 byte int. vec, edge triggered. 
#define ICW1_AT     0x11
// ICW4 needed, cascade, 8 byte int. vec., level triggered.
#define ICW1_PS2    0x19

//Define at which vector IRQ0-7/IRQ8-15 tables start
#define INT1_ICW2   IRQ0_VECTOR
#define INT2_ICW2   IRQ8_VECTOR

// Defines which IRQ connects slave/master 
#define INT1_ICW3   (1 << CASCADE_IRQ)
#define INT2_ICW3   CASCADE_IRQ

// 80x86 mode
#define ICW4        0x01


//Variables
//irq_handler_t   InterruptHandler::irq_table[NO_OF_IRQ_VECTORS];
int             InterruptHandler::irq_usemap = 0;
int             InterruptHandler::unlocking_allowed = FALSE;

// Initialize interrupts
// (no kidding... =)

int InterruptHandler::initialize (void)
{
  Console::print ("Initializing interrupt handler...");

  init_i8259();
  
  //Set all vectors to default (spurious)
  for (int i = 0; i < NO_OF_IRQ_VECTORS; i++)
    InterruptHandler_irq_table[i] = default_irq_handler;

  Console::println ("done");
  unlocking_allowed = TRUE;
  return 0;
}

//
// initialize i8259 interrupt controllers (very x86 specific)
//

void InterruptHandler::init_i8259 (void)
{
  // Should check for PS/2 here, and replace ICW1_AT
  ::out_byte (INT1_CTL, ICW1_AT);

  ::out_byte (INT1_CTLMASK, INT1_ICW2);
  ::out_byte (INT1_CTLMASK, INT1_ICW3);
  ::out_byte (INT1_CTLMASK, ICW4);

  //Mask off all interrupts but cascade (OCW1)
  ::out_byte (INT1_CTLMASK, ~(1 << CASCADE_IRQ));

  //Master controller done, do the same, almost, to the slave.
  //(should check ps/2 here too)
  ::out_byte (INT2_CTL, ICW1_AT);
  
  ::out_byte (INT2_CTLMASK, INT2_ICW2);
  ::out_byte (INT2_CTLMASK, INT2_ICW3);
  ::out_byte (INT2_CTLMASK, ICW4);

  //Mask off all interrupts for slave controller
  ::out_byte (INT2_CTLMASK, ~0);
}

//
// default irq handler, should print out some complaint
//

int InterruptHandler::default_irq_handler (int irq)
{
  //Do nothing for now... (should do sanity checks on irq and panic)
  return 1;
}

//
// lock, aka cli
//

void InterruptHandler::lock (void)
{
  __asm__ ("cli");
}

//
// unlock, aka sti
//

void InterruptHandler::unlock (void)
{
  if (unlocking_allowed)
    __asm__ ("sti");
}

//
// register_handler, sets up an irq handler
//

int InterruptHandler::register_handler (irq_handler_t handler, int irq)
{
  if (InterruptHandler_irq_table[irq] == handler) //re-registered, allowed!
    return 0;

  if (irq < 0 || irq >= NO_OF_IRQ_VECTORS)
  {    
    return 1;
  } else if (InterruptHandler_irq_table[irq] != default_irq_handler)
  {
    //attempt to register second handler, forbidden!
    return 2;
  } else
  {
    disable_irq(irq);
    InterruptHandler_irq_table [irq] = handler;
    irq_usemap |= 1 << irq;

    return 0;
  }
}

//
// disable irq, masks of an irq
//

void InterruptHandler::disable_irq (int irq)
{
  //Have to preserve flags register...

  lock ();
  if (irq > 0 && irq < 8)
  {
    __asm__ ("pushf");
    ::out_byte (INT1_CTLMASK, ::in_byte (INT1_CTLMASK) | (1 << irq));
    __asm__ ("popf");
  }
  else if (irq > 7 && irq < 16)
  {
    __asm__ ("pushf");
    ::out_byte (INT2_CTLMASK, ::in_byte (INT2_CTLMASK) | (1 << (irq - 8)));    
    __asm__ ("popf");
  }
}


//
// enable irq, masks on an irq
//

void InterruptHandler::enable_irq (int irq)
{
  //Have to preserve flags register...

  lock ();
  if (irq > 0 && irq < 8)
  {
    __asm__ ("pushf");
    ::out_byte (INT1_CTLMASK, ::in_byte (INT1_CTLMASK) & ~(1 << irq));
    __asm__ ("popf");
  }
  else if (irq > 7 && irq < 16)
  {
    __asm__ ("pushf");
    ::out_byte (INT2_CTLMASK, ::in_byte (INT2_CTLMASK) & ~(1 << (irq - 8)));    
    __asm__ ("popf");
  }
}

void InterruptHandler::unhold (void)
{
  p_elem_t   *held;

  if (Scheduler::switching) return;

  held = InterruptHandler_held_int_head;
  do
  {
    if ((InterruptHandler_held_int_head = held->next_held_int) == NIL_PROC)
      InterruptHandler_held_int_tail = NIL_PROC;

    held->int_held = FALSE;
    unlock();  //Reduces latency
    
    //Send interrupt MESSAGE to process
    //interrupt (held->process_id)

    lock();
  } while ((held = InterruptHandler_held_int_head) != NIL_PROC);
}

//
// Processor Exception handler
//

void InterruptHandler::exception (unsigned  ex_nr)
{
  struct exception_s
  {
    char   *message;
    int     signal_nr;
  };

  static struct exception_s exception_data[] = {
    {"Divide error", SIGFPE},
    {"Debug exception", SIGTRAP},
    {"Nonmaskable interrupt", SIGBUS},
    {"Breakpoint", SIGEMT},
    {"Overflow", SIGFPE},
    {"Bounds check", SIGFPE},
    {"Invalid opcode", SIGILL},
    {"Coprocessor not availiable", SIGFPE},
    {"Double fault", SIGBUS},
    {"Coprocessor segment overrun", SIGSEGV},
    {"Invalid TSS", SIGSEGV},
    {"Segment not present", SIGSEGV},
    {"Stack exception", SIGSEGV},
    {"General protection", SIGSEGV},
    {"Page fault", SIGSEGV},
    {NIL_PTR, SIGILL},      //Reserved by intel
    {"Coprocessor error", SIGFPE}
  };
    
  struct exception_s   *ex_p;
  p_elem_t             *cur_proc;

  //Save proc ptr, since it might get changed
  cur_proc = Scheduler_current_proc_ptr;

  ex_p = &exception_data[ex_nr];

  //Send a signal (message) to the process
  // Not implemented yet, but outline should be clear from this:

  //if (InterruptHandler_k_reenter == 0 && cur_proc->is_user_process)
  //{
  //  unlock();
  //  send_signal (cur_proc->pid, ex_p->signal_nr);
  //  return;
  //}
  
  //Do this for now!
  Console::print ("Exception: ");
  Console::println (ex_p->message);
}



