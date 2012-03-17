//
// scheduler.cpp
// This is the scheduler class for my O/S project
// 
// (c) Jimmy Larsson 1998
//

#include "constants.h"
#include "types.h"
#include "scheduler.h"
#include "console.h"

//This is a trick to let assembler code access these var/functs.
//Really ugly!
p_elem_t   *Scheduler_current_proc_ptr;

/* space for queue elements */
static p_elem_t   p_queue_elements[MAX_PROCESSES];


int         Scheduler::switching;
p_elem_t   *Scheduler::p_queue;
p_elem_t   *Scheduler::p_queue_tail;


int Scheduler::initialize (void)
{
  Console::print ("Initializing scheduler...");

  //Setup task queues, and other data
  p_queue = &p_queue_elements[0];
  p_queue_tail = p_queue;
  switching = FALSE;

  Console::println (" done");
  return 0;
}

void Scheduler::idle_task (void)  // The IDLE task
{
  //Do something like 'halt'... =)
}


//
// Decide if it's time for a new task to run
//

void Scheduler::schedule (void)
{ 
  p_elem_t   *task_to_run;

  if (p_queue != NULL)          //If there is a task select it
  {
    task_to_run = p_queue;
    
    if (p_queue->next != NULL)  //If there is another task, put it first in queue
    {
      p_queue = p_queue->next;
      task_to_run->previous = p_queue_tail;
      p_queue_tail = task_to_run;
      task_to_run->next = NULL;
    }
  } else   //No task, run idle task
  {
    idle_task ();
  }
  
  //Here the assembler routine to swap EIP and other registers should be called
}
    








