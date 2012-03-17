//
// console.cc
// This is the console class for my O/S project
// 
// (c) Jimmy Larsson 1998
//
// This "console" is *only* used when booting system, before
// graphics has been fired up. Later I might remove it and let 
// the graphics classes take care of it...
//
// 
//

#define DEFAULT_CGA_LINE_WIDTH 80
#define DEFAULT_CGA_HEIGHT     25

#include "interrupt.h"
#include "console.h"


int   Console::cursor_x;
int   Console::cursor_y;
int   Console::attributes;
int   Console::line_width;
int   Console::height;

extern "C" void printmess(char *mess, int x, int y, int attr);

//
// Initialize console, with 80x25 CGA textmode for now...
//

void Console::initialize (void)
{
  cursor_x = 0;
  cursor_y = 0;
  attributes = 0x1f;
  line_width = DEFAULT_CGA_LINE_WIDTH;
  height = DEFAULT_CGA_HEIGHT;
  clear();
}

//
// Prints null-terminated string to screen with line wrap,
// !!NO checking for write past screen, must have!!
//
void Console::printString (char   *string)
{
  static char   *tmp_ptr;

  ::printmess (string, cursor_x, cursor_y, attributes);

  tmp_ptr = string;

  while (*tmp_ptr != '\0')
    tmp_ptr++;

  cursor_x = cursor_x + (tmp_ptr - string);
  
  while (cursor_x >= line_width)
  {
    cursor_x = cursor_x - line_width;
    cursor_y++;
  }

  while (cursor_y >= height)
  {
    cursor_x = 0;
    cursor_y = 0;
  }

}  


void Console::print (char   *message)
{
  InterruptHandler::lock();
  printString (message);
  InterruptHandler::unlock();
}

//
//Print longword in hex format
//
void Console::print (long   number)
{
  char   tmp_str[2];
  long   tmp;

  InterruptHandler::lock();
  printString ("0x");
  tmp_str[1] = 0;

  for (int x = 7; x >= 0; x--)
  {
    tmp = (number & (0xf << x*4));
    tmp = tmp >> (x*4);

    if (tmp > 9)
      tmp_str[0] = (tmp - 10) + 'a';
    else
      tmp_str[0] = tmp + '0';

    printString (tmp_str);
  }
  InterruptHandler::unlock();
}


//
// Prints null-terminated string and then breaks line
//

void Console::println (char   *mess)
{
  InterruptHandler::lock();
  printString (mess);

  //Do a  "linefeed"... =)
  cursor_y++;
  cursor_x = 0;
  InterruptHandler::unlock();
}

//
// Clear screen
//

void Console::clear (void)
{
  static int   i;

  InterruptHandler::lock();
  for (i = 0; i < (line_width * height); i++)
  {
    printString (" ");
  }
  cursor_x = 0;
  cursor_y = 0;
  InterruptHandler::unlock();
}






