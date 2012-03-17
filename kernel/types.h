//
// types.h
// type declarations for whole system
// (c) Jimmy Larsson
//
//

#ifndef _TYPES_H
#define _TYPES_H

//Various types
typedef unsigned char       u8_t;
typedef unsigned short      u16_t;
typedef unsigned int        u32_t;

typedef unsigned            reg_t;
typedef unsigned            segm_t;
typedef unsigned            port_t;

typedef unsigned char       uchar;
typedef unsigned short      ushort; 

typedef unsigned long       phys_bytes_t;
typedef unsigned long       vir_bytes_t;

//irq handler type
typedef int (*irq_handler_t)(int irq);

//Message type
typedef struct message_s
{
  int     source;
  long    data1;
  long    data2;
  long    data3;
  void   *ptr1;
  void   *ptr2;
  void   *ptr3;
} message_t;


// GDT, LDT and IDT pointer type
typedef struct descriptor_table_ptr_s    
{
  char   limit [sizeof (u16_t)];  // table limit
  char   base [sizeof (u32_t)];   // table base address
} descriptor_table_ptr_t;


// call gate descriptor
typedef struct gate_descriptor_s         
{
  u16_t   offset_low;
  u16_t   selector;
  u8_t    pad;           // |000xxxxx| for int. gate and trap gate, 
                         // |xxxxxxxx| for task gate
  u8_t    p_dpl_type;    // P,DPL,0,TYPE on the form:  |x|xx|0|xxxx|
  u16_t   offset_high;
} gate_descriptor_t;


// segment descriptor type
typedef struct segment_descriptor_s
{
  u16_t   limit_low;
  u16_t   base_low;
  u8_t    base_middle;
  u8_t    access;          // |P|DL|1|x|E|R|A|,  DL = dpl
  u8_t    granularity;     // |G|x|0|A|LIMT|
  u8_t    base_high;
} segment_descriptor_t;

  
//task state segment type, actually only used once, but do it this way
// (nice and uniform)
typedef struct tss_s
{
  reg_t   backlink;
  reg_t   sp0;
  reg_t   ss0;
  reg_t   sp1;
  reg_t   ss1;
  reg_t   sp2;
  reg_t   ss2;
  reg_t   cr3;
  reg_t   ip;
  reg_t   flags;
  reg_t   ax;
  reg_t   cx;
  reg_t   dx;
  reg_t   bx;
  reg_t   sp;
  reg_t   bp;
  reg_t   si;
  reg_t   di;
  reg_t   es;
  reg_t   cs;
  reg_t   ss;
  reg_t   ds;
  reg_t   fs;
  reg_t   gs;
  reg_t   ldt;
  u16_t   trap;
  u16_t   iobase;
} tss_t;


//
// Stackframe, this is pushed by CPU and save() on interrupt
//

typedef struct stackframe_s 
{                               // current_proc_ptr points here
  u16_t gs;                     // last item pushed by save
  u16_t fs;                     
  u16_t es;                     
  u16_t ds;                   
  reg_t di;                     // di through cx are not accessed in C
  reg_t si;                     // order is to match pusha/popa
  reg_t fp;                     // bp 
  reg_t st;                     // hole for another copy of sp
  reg_t bx;                     
  reg_t dx;                     
  reg_t cx;                    
  reg_t retreg;                 // ax and above are all pushed by save function
  reg_t retadr;                 // return address for assembly code save()
  reg_t pc;                    
  reg_t cs;                    
  reg_t psw;                    
  reg_t sp;                     
  reg_t ss;                     
} stackframe_t;
                                 


// Process queue elements
typedef struct p_elem_s
{
  //This has to be first, simplifies asm code...
  stackframe_t       cpu_regs;
  struct p_elem_s   *next;
  struct p_elem_s   *previous;
  struct p_elem_s   *next_held_int;   //Links held interrupts
  int                int_held;        //Interrupt held flag
  long               process_id;
  long               parent;
} p_elem_t;



#endif





