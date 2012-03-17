//
// main.cc
// You can never guess what this is... =)
// (c) Jimmy Larsson 1998
//

#include "../kernel/descriptor.h"
#include "../kernel/interrupt.h"
#include "../kernel/scheduler.h"
#include "../kernel/console.h"
#include "../kernel/io_ports.h"
#include "../kernel/sysinfo.h"

//
// test interrupt code
//

int printClock (int irq)
{
  Console::print ("!");
  return 1;
}


// Startup the whole O/S (when we enter, interrupts are disabled)
//
// 1. Initialize scheduler and process queues
// 2. Setup RTC
// 3. Initialize/Enable interrupts
//
// (servers and services initializes themselves)
//

int main (void)
{
  rtc_t *time;

  Console::initialize();
  DescriptorHandler::initialize();
  SysInfo::initialize();
  Scheduler::initialize();
  InterruptHandler::initialize(); 
  time = SysInfo::getCMOSClock();

  Console::print ((long) time->year);

  //Test with simple clock interrupt
  InterruptHandler::lock();
  out_byte (0x43, 0x36);
  out_byte (0x40, 0xff);
  out_byte (0x40, 0xfe);

  InterruptHandler::register_handler (printClock, 0);
  InterruptHandler::enable_irq(0);
  InterruptHandler::unlock();
  
  __asm__ ("int $0x28");


}


