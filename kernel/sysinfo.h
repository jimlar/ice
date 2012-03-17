//
// sysinfo.h
// This is the system detection class for my O/S project
// (Memory size, harddisks, etc...
//
// (c) Jimmy Larsson 1998
//

#ifndef _SYSINFO_H
#define _SYSINFO_H

#include "types.h"

typedef struct rtc_s
{
  uchar   seconds;
  uchar   minutes;
  uchar   hours;
  uchar   day_of_week;
  uchar   day_of_month;
  uchar   month;
  int     year;
} rtc_t;


class SysInfo
{
 private:
  static int base_mem_size;
  static long extended_mem_size;

  static uchar bcd2bin (uchar bcd);
  static int bcd2bin (int bcd);

 public:
  static void initialize (void);     // Detect hardware
  static rtc_t *getCMOSClock (void); // Read RTC from CMOS

};

#endif
