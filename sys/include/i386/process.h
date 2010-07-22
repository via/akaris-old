#ifndef I386_PROCESS_H
#define I386_PROCESS_H

#include <i386/types.h>
#include <i386/mailbox.h>
#include <i386/interrupt.h>
#include <i386/kqueue.h>
/*Process entries*/


#define PROCESS_STATUS_RUNNING 1
#define PROCESS_STATUS_WAITING 2
#define PROCESS_STATUS_YIELDING 3

/*Forward decl*/
struct address_space;

typedef struct context {
  isr_regs registers;
  struct address_space * space;
  uint32 pid;
  int status;
  struct context * next;
  uint32 kqueue_block;
} context_t;


void initialize_scheduler();
void schedule(isr_regs *);
int create_process();
int execve_elf (context_t *, void *, unsigned long, const char *);
int fork (isr_regs *);
context_t * get_process(uint32);
void begin_schedule (isr_regs *);
uint32 get_current_process();
void set_current_process (int);
#endif
