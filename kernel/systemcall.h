//
// systemcall.h
// This is the system call handling class for my O/S project
// 
// (c) Jimmy Larsson 1998
//

#ifndef _SYSTEMCALL_H
#define _SYSTEMCALL_H

#include "types.h"

class SystemCall
{
 private:
 
 public:
  
  static int call (int function, int src_dest, message_t *message);
 
};

#endif



