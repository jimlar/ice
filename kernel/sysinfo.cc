//
// sysinfo.h
// This is the system detection class for my O/S project
// (Memory size, harddisks, etc...
//
// (c) Jimmy Larsson 1998
//

#include "sysinfo.h"
#include "io_ports.h"
#include "console.h"
#include "interrupt.h"

#define CMOS_ADDRESS_PORT 0x70
#define CMOS_DATA_PORT    0x71

#define CMOS_BASEMEM_LSB  0x15
#define CMOS_BASEMEM_MSB  0x16
#define CMOS_EXTMEM_LSB   0x17
#define CMOS_EXTMEM_MSB   0x18

#define CMOS_CLOCK_SECS    0x00
#define CMOS_CLOCK_MINS    0x02
#define CMOS_CLOCK_HOURS   0x04
#define CMOS_CLOCK_DOW     0x06
#define CMOS_CLOCK_DOM     0x07
#define CMOS_CLOCK_MONTH   0x08
#define CMOS_CLOCK_YEAR    0x09
#define CMOS_CLOCK_CENTURY 0x32
#define CMOS_RTC_STATUS_A  0x0a
#define CMOS_RTC_STATUS_B  0x0b

//Variables
int  SysInfo::base_mem_size;
long SysInfo::extended_mem_size;


//
// initialize, detects hardware
//
void SysInfo::initialize (void)
{
  InterruptHandler::lock();  //NOT REENTRANT!

  //Detect base memory
  ::out_byte (CMOS_ADDRESS_PORT, CMOS_BASEMEM_LSB);
  base_mem_size = ::in_byte (CMOS_DATA_PORT);
  ::out_byte (CMOS_ADDRESS_PORT, CMOS_BASEMEM_MSB);
  base_mem_size += (::in_byte (CMOS_DATA_PORT) * 0xff);

  ::out_byte (CMOS_ADDRESS_PORT, CMOS_EXTMEM_LSB);
  extended_mem_size = ::in_byte (CMOS_DATA_PORT);
  ::out_byte (CMOS_ADDRESS_PORT, CMOS_EXTMEM_MSB);
  extended_mem_size += (::in_byte (CMOS_DATA_PORT) * 0xff);

  Console::print ("Found ");
  Console::print ((long) base_mem_size);
  Console::print ("k base, ");
  Console::print (extended_mem_size);
  Console::println ("k extended memory");

  InterruptHandler::unlock();
}


//
// getCMOSClock
// reads RTC from CMOS memory and returns it
//
rtc_t   *SysInfo::getCMOSClock (void)
{
  static rtc_t   return_rtc;
  int            bcd;

  InterruptHandler::lock();  //NOT REENTRANT!

  //Time/date in BCD format?
  ::out_byte (CMOS_ADDRESS_PORT, CMOS_RTC_STATUS_B);
  bcd = !(::in_byte (CMOS_DATA_PORT) & 0x04);


  // Wait if there's an update going on
  while (1)
  {
    ::out_byte (CMOS_ADDRESS_PORT, CMOS_RTC_STATUS_A);
    if ((::in_byte (CMOS_DATA_PORT) & 0x80) == 0)
      break;
  }

  ::out_byte (CMOS_ADDRESS_PORT, CMOS_CLOCK_SECS);
  return_rtc.seconds = ::in_byte (CMOS_DATA_PORT);

  ::out_byte (CMOS_ADDRESS_PORT, CMOS_CLOCK_MINS);
  return_rtc.minutes = ::in_byte (CMOS_DATA_PORT);

  ::out_byte (CMOS_ADDRESS_PORT, CMOS_CLOCK_HOURS);
  return_rtc.hours = ::in_byte (CMOS_DATA_PORT);

  ::out_byte (CMOS_ADDRESS_PORT, CMOS_CLOCK_DOW);
  return_rtc.day_of_week = ::in_byte (CMOS_DATA_PORT);

  ::out_byte (CMOS_ADDRESS_PORT, CMOS_CLOCK_DOM);
  return_rtc.day_of_month = ::in_byte (CMOS_DATA_PORT);

  ::out_byte (CMOS_ADDRESS_PORT, CMOS_CLOCK_MONTH);
  return_rtc.month = ::in_byte (CMOS_DATA_PORT);

  ::out_byte (CMOS_ADDRESS_PORT, CMOS_CLOCK_YEAR);
  return_rtc.year = ::in_byte (CMOS_DATA_PORT);

  if (bcd)
  {
    return_rtc.seconds = bcd2bin (return_rtc.seconds);
    return_rtc.minutes = bcd2bin (return_rtc.minutes);
    return_rtc.hours = bcd2bin (return_rtc.hours);
    return_rtc.day_of_week = bcd2bin (return_rtc.day_of_week);
    return_rtc.day_of_month = bcd2bin (return_rtc.day_of_month);
    return_rtc.month = bcd2bin (return_rtc.month);
    return_rtc.year = bcd2bin (return_rtc.year);
  }

  //Century fixup year < 50 = 2000, year >= 50 = 1900 
  // (CMOS century byte dosen't work, AFAIK)
  if (return_rtc.year >= 50)
    return_rtc.year += 1900;
  else
    return_rtc.year += 2000;

  InterruptHandler::unlock();
  return &return_rtc;
}

//
// bcd2bin, convert bcd number to "binary" number
//
uchar SysInfo::bcd2bin (uchar bcd)
{
  uchar   tmp;

  tmp = bcd & 0x0f;
  tmp += ((bcd & 0xf0) >> 4) * 10;

  return tmp;
}
  

//
// bcd2bin, convert bcd number to "binary" number
//
int SysInfo::bcd2bin (int bcd)
{
  int   tmp;

  tmp = bcd & 0x000f;
  tmp += ((bcd & 0x00f0) >> 4) * 10;
  tmp += ((bcd & 0x0f00) >> 8) * 10;
  tmp += ((bcd & 0xf000) >> 12) * 10;

  return tmp;
}
