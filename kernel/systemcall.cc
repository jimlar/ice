//
// systemcall.h
// This is the system call handling class for my O/S project
// 
// (c) Jimmy Larsson 1998
//

#include "systemcall.h"
#include "types.h"

//Call handler
int (*SystemCall_call)(int, int, message_t*) = SystemCall::call;



//
// Actual system call handler function
//
int SystemCall::call (int function, int src_dest, message_t *message)
{

  return 0;
}
