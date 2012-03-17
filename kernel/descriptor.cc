//
// descriptor.cc
//
// Handles GDT, IDT and TSS
// (c) 1998 Jimmy Larsson
//
//

#include "types.h"
#include "constants.h"
#include "console.h"
#include "interrupt.h"
#include "descriptor.h"

//This is a trick to let assembler code access these var/functs.
//Really ugly!
tss_t  DescriptorHandler_tss;


//External variables, from kickstart.S 
extern "C" segment_descriptor_t   _gdt;
extern "C" gate_descriptor_t      _idt;

//External interrupt stubs (interrupt_386.S)
extern "C" void hardware_int_00 (void);
extern "C" void hardware_int_01 (void);
extern "C" void hardware_int_02 (void);
extern "C" void hardware_int_03 (void);
extern "C" void hardware_int_04 (void);
extern "C" void hardware_int_05 (void);
extern "C" void hardware_int_06 (void);
extern "C" void hardware_int_07 (void);
extern "C" void hardware_int_08 (void);
extern "C" void hardware_int_09 (void);
extern "C" void hardware_int_10 (void);
extern "C" void hardware_int_11 (void);
extern "C" void hardware_int_12 (void);
extern "C" void hardware_int_13 (void);
extern "C" void hardware_int_14 (void);
extern "C" void hardware_int_15 (void);

//External exception stubs (interrupt_386.S)
extern "C" void exception_divide (void);
extern "C" void exception_step (void);
extern "C" void exception_nmi (void);
extern "C" void exception_breakpoint (void);
extern "C" void exception_overflow (void);
extern "C" void exception_invalid_opcode (void);
extern "C" void exception_bounds_check (void);
extern "C" void exception_coproc_not_availiable (void);
extern "C" void exception_double_fault (void);
extern "C" void exception_coproc_seg_overrun (void);
extern "C" void exception_invalid_tss (void);
extern "C" void exception_segment_not_present (void);
extern "C" void exception_stack (void);
extern "C" void exception_general_protection (void);
extern "C" void exception_page_fault (void);
extern "C" void exception_coproc_error (void);

extern "C" void system_call (void);

//Variables

//Simple GDT pointer (to first descriptor)
segment_descriptor_t  *DescriptorHandler::gdt;

//pointer to first IDT gate
gate_descriptor_t     *DescriptorHandler::idt;

//kernel data base address
phys_bytes_t           DescriptorHandler::data_base;

//TSS, the only one used, stored here
tss_t                 *DescriptorHandler::tss = &DescriptorHandler_tss;

//
// initialize descriptors/tables
// (partially already done by kickstart.S)
//

void DescriptorHandler::initialize (void)
{
  descriptor_table_ptr_t   *dtp;

  Console::print ("Initializing descriptors/TSS... ");

  //pointer to GDT/IDT setup by kickstart.S, simpler this way
  gdt = &::_gdt;
  idt = &::_idt;

  Console::print ("GDT: ");
  Console::print ((long) gdt);
  Console::print (" IDT: ");
  Console::print ((long) idt);
  Console::print (" ");

  //Read data base address
  data_base = gdt[DS_INDEX].base_low;
  data_base += gdt[DS_INDEX].base_middle << BASE_MIDDLE_SHIFT;
  data_base += gdt[DS_INDEX].base_high << BASE_HIGH_SHIFT;

  //Put GDT and IDT selectors in GDT (bios want's this, in case we want
  //to use bios int's for HD or so...)
  dtp = (descriptor_table_ptr_t *) &gdt[GDT_INDEX];
  *(u16_t *) dtp->limit = GDT_SIZE * DESCRIPTOR_SIZE - 1;
  *(u32_t *) dtp->base = virtual2physical((vir_bytes_t) gdt);

  dtp = (descriptor_table_ptr_t *) &gdt[IDT_INDEX];
  *(u16_t *) dtp->limit = IDT_SIZE * DESCRIPTOR_SIZE - 1;
  *(u32_t *) dtp->base = virtual2physical((vir_bytes_t) idt);

  //Setup descriptor for system processes (e.g. drivers, servers)
  init_data_seg (&gdt[ES_INDEX], 0L, 0L, SYS_PROC_PRIVILEGE); 

  //Setup TSS 
  tss->ss0 = DS_SELECTOR;
  init_data_seg (&gdt[TSS_INDEX], virtual2physical((vir_bytes_t) tss), (phys_bytes_t) sizeof (*tss), INT_PRIVILEGE);
  gdt[TSS_INDEX].access = PRESENT | (INT_PRIVILEGE << DPL_SHIFT) | TSS_TYPE;
  tss->iobase = sizeof (tss);

  //Load taskregister with TSS_SELECTOR
  __asm__("mov $0x40, %ax
           ltr %ax");

  //Setup hardware int vectors
  add_int_gate (IRQ0_VECTOR, (phys_bytes_t) hardware_int_00, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (IRQ0_VECTOR+1, (phys_bytes_t) hardware_int_01, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (IRQ0_VECTOR+2, (phys_bytes_t) hardware_int_02, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (IRQ0_VECTOR+3, (phys_bytes_t) hardware_int_03, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (IRQ0_VECTOR+4, (phys_bytes_t) hardware_int_04, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (IRQ0_VECTOR+5, (phys_bytes_t) hardware_int_05, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (IRQ0_VECTOR+6, (phys_bytes_t) hardware_int_06, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (IRQ0_VECTOR+7, (phys_bytes_t) hardware_int_07, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (IRQ8_VECTOR, (phys_bytes_t) hardware_int_08, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (IRQ8_VECTOR+1, (phys_bytes_t) hardware_int_09, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (IRQ8_VECTOR+2, (phys_bytes_t) hardware_int_10, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (IRQ8_VECTOR+3, (phys_bytes_t) hardware_int_11, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (IRQ8_VECTOR+4, (phys_bytes_t) hardware_int_12, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (IRQ8_VECTOR+5, (phys_bytes_t) hardware_int_13, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (IRQ8_VECTOR+6, (phys_bytes_t) hardware_int_14, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (IRQ8_VECTOR+7, (phys_bytes_t) hardware_int_15, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));

  //Setup exception vectors
  add_int_gate (DIVIDE_INT, (phys_bytes_t) ::exception_divide, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (DEBUG_INT, (phys_bytes_t) ::exception_step, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (NMI_INT, (phys_bytes_t) ::exception_nmi, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (BREAKPOINT_INT, (phys_bytes_t) ::exception_breakpoint, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (OVERFLOW_INT, (phys_bytes_t) ::exception_overflow, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (BOUNDS_CHECK_INT, (phys_bytes_t) ::exception_bounds_check, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (INVALID_OPCODE_INT, (phys_bytes_t) ::exception_invalid_opcode, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (COPROC_NOT_AVAIL_INT, (phys_bytes_t) ::exception_coproc_not_availiable, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (DOUBLE_FAULT_INT, (phys_bytes_t) ::exception_double_fault, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (COPROC_SEG_OVERRUN_INT, (phys_bytes_t) ::exception_coproc_seg_overrun, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (INVALID_TSS_INT, (phys_bytes_t) ::exception_invalid_tss, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (SEGMENT_NOT_PRESENT_INT, (phys_bytes_t) ::exception_segment_not_present, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (STACK_FAULT_INT, (phys_bytes_t) ::exception_stack, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (GENERAL_PROTECTION_INT, (phys_bytes_t) ::exception_general_protection, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (PAGE_FAULT_INT, (phys_bytes_t) ::exception_page_fault, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));
  add_int_gate (COPROC_ERROR_INT, (phys_bytes_t) ::exception_coproc_error, PRESENT | INT_GATE_TYPE | (INT_PRIVILEGE << DPL_SHIFT));

  //System call vector
  add_int_gate (SYS_CALL_INT, (phys_bytes_t) ::system_call, PRESENT | INT_GATE_TYPE | (USER_PRIVILEGE << DPL_SHIFT));

  Console::println (" done");
}

//
// init_seg_desc, initilaize a segement descriptor with
// base, limit and granularity
//

void DescriptorHandler::init_seg_desc (segment_descriptor_t *seg_desc, phys_bytes_t base, phys_bytes_t size)
{
  seg_desc->base_low = base;
  seg_desc->base_middle = base << BASE_MIDDLE_SHIFT;
  seg_desc->base_high = base >> BASE_HIGH_SHIFT;
  size--; //make size a limit
  if (size > BYTE_GRAN_MAX)
  {
    seg_desc->limit_low = size >> PAGE_GRAN_SHIFT;
    seg_desc->granularity = GRANULAR | (size >> (PAGE_GRAN_SHIFT + GRANULARITY_SHIFT));
  } else
  {
    seg_desc->limit_low = size;
    seg_desc->granularity = size >> GRANULARITY_SHIFT;
  }
  seg_desc->granularity |= DEFAULT_GRANULARITY;
}

//
// init_data_seg, initilaizes a data segment descriptor
//

void DescriptorHandler::init_data_seg (segment_descriptor_t *seg_desc, phys_bytes_t base, phys_bytes_t size, int privilege)
{
  init_seg_desc (seg_desc, base, size);
  seg_desc->access = (privilege << DPL_SHIFT) | (PRESENT | SEGMENT | WRITEABLE);
}

//
// virtual2physical, translate virtual address to physical
//

phys_bytes_t DescriptorHandler::virtual2physical (vir_bytes_t vir_addr)
{
  return (phys_bytes_t) vir_addr + data_base;
}

//
// add_int_gate, setup interrupt callgate in IDT
//

void DescriptorHandler::add_int_gate (unsigned vector, phys_bytes_t base, unsigned dpl_type)
{
  gate_descriptor_t   *idp;

  idp = &idt[vector];
  idp->offset_low = base;
  idp->offset_high = base >> OFFSET_HIGH_SHIFT;
  idp->selector = CS_SELECTOR;
  idp->p_dpl_type = dpl_type;
}
