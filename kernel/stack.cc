//
// kernel stack definitions
// (c) Jimmy Larsson 1998
//

#include "constants.h"
#include "descriptor.h"

// kernel stack space
long stack[KERNEL_STACK_SIZE];

// kernel stack pointer, used in assm. code
struct stack_ptr 
{
  long * a;
  short b;
} stack_start = {&stack [KERNEL_STACK_SIZE], DS_SELECTOR};            

