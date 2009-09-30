#ifndef I386_PROCESS_H
#define I386_PROCESS_H

/*Process entries*/

#include <i386/types.h>
#include <i386/interrupt.h>
#include <i386/context.h>
#include <i386/slab.h>

#define PROCESS_STATUS_RUNNING 1
#define PROCESS_STATUS_WAITING 2
#define PROCESS_STATUS_YIELDING 3

struct context {
  isr_regs registers;
  address_space * space;
  int pid;
  int status;
  struct context * next;
};

typedef struct context context_t;

void initialize_scheduler();
void schedule(isr_regs *);
int create_process(int addr, int length);
context_t * get_process(int);
void begin_schedule (isr_regs *);
int get_current_process();
#endif
