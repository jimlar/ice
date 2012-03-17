//
// scheduler.h
// This is the scheduler class for my O/S project
// 
// (c) Jimmy Larsson 1998
//

#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include "types.h"

//Trick for asm code
extern p_elem_t   *Scheduler_current_proc_ptr;

class Scheduler 
{
 private:
  static p_elem_t   *p_queue;         //Process queue head pointer
  static p_elem_t   *p_queue_tail;    //Process queue tail pointer
  
  static void idle_task (void);     //The idle task

 public:
  static int switching;  //Sched, semaphore

  static int initialize (void);     // Startup scheduler
  static void schedule (void);      // toss in next task

};

#endif
