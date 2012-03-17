//
// io_ports.h
// One of the few non c++ files in my os, low-level port i/o
// 
// Should really put these in some class to, but don't know where...
// Well, no time now... =)
//
// (c) Jimmy Larsson 1998
//

#ifndef _IO_PORTS_H
#define _IO_PORTS_H

#include "types.h"

extern "C" void out_byte (port_t port, u8_t data);
extern "C" unsigned in_byte (port_t);

#endif
