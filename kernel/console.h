//
// console.h
// This is the console class for my O/S project
// 
// (c) Jimmy Larsson 1998
//
// This "console" is *only* used when booting system, before
// graphics has been fired up. Later I might remove it and let 
// the graphics object take care of it...
//
//

#ifndef _CONSOLE_H
#define _CONSOLE_H

#include "types.h"

class Console
{
 private:
  static int   cursor_x;
  static int   cursor_y;
  static int   attributes;
  static int   line_width;
  static int   height;

  static void  printString (char   *string);

 public:
  static void initialize (void);
  static void print (char   *message);
  static void print (long    number);
  static void println (char *message);
  static void clear (void);

};

#endif
