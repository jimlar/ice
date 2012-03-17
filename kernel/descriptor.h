//
// descriptor.h
//
// Handles GDT, IDT and TSS
// Also has some basic address-translation member functions
//
// (c) 1998 Jimmy Larsson
//
//

#ifndef _DESCRIPTOR_H
#define _DESCRIPTOR_H

#include "types.h"

//These MUST match kickstart.S
#define GDT_SIZE 256
#define IDT_SIZE 256
#define LDT_SIZE 2

//Size of one descriptor (bytes)
#define DESCRIPTOR_SIZE 8

//GDT fixed indexes, 1-7 are demanded by BIOS
#define GDT_INDEX        1
#define IDT_INDEX        2
#define DS_INDEX         3
#define ES_INDEX         4
#define SS_INDEX         5
#define CS_INDEX         6
#define BIOS_TMP_INDEX   7

// TSS selector index
#define TSS_INDEX        8

//GDT Selectors
#define GDT_SELECTOR  0x08
#define IDT_SELECTOR  0x10
#define DS_SELECTOR   0x18
#define ES_SELECTOR   0x20
#define SS_SELECTOR   0x28
#define CS_SELECTOR   0x30
#define TSS_SELECTOR  0x40

//Descriptor constants
#define BASE_MIDDLE_SHIFT   16 
#define BASE_HIGH_SHIFT     24
#define PAGE_GRAN_SHIFT     12
#define BYTE_GRAN_MAX       0xfffffL
//4k granualrity
#define GRANULAR            0x80
#define GRANULARITY_SHIFT   16
#define DEFAULT_GRANULARITY 0x40

#define OFFSET_HIGH_SHIFT   16

#define PRESENT             0x80
#define DPL_SHIFT           5
#define SEGMENT             0x10
#define WRITEABLE           0x02

#define AVL_286_TSS         1
//OR with this for 386 type
#define DESC_386_BIT        0x08
//Interrupt gate, for all vectors
#define INT_286_GATE        6
#define TSS_TYPE            (AVL_286_TSS | DESC_386_BIT)
#define INT_GATE_TYPE       (INT_286_GATE | DESC_386_BIT)

class DescriptorHandler
{
 private:
  static phys_bytes_t              data_base;
  static segment_descriptor_t     *gdt;
  static gate_descriptor_t        *idt;

  static void init_seg_desc (segment_descriptor_t *seg_desc, phys_bytes_t base, phys_bytes_t size);

 public:
  static tss_t *tss; //Task state segment

  static void initialize (void);  //Setup
  //Create an interrupt gate, for both IRQ's and INT's.
  static void add_int_gate (unsigned vector, phys_bytes_t base, unsigned dpl_type);
  static void init_data_seg (segment_descriptor_t *seg_desc, phys_bytes_t base, phys_bytes_t size, int privilege);

  //Address translations
  static phys_bytes_t virtual2physical (vir_bytes_t vir_addr);
};


#endif






