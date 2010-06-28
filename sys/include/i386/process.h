#ifndef I386_PROCESS_H
#define I386_PROCESS_H

#include <i386/mailbox.h>
#include <i386/interrupt.h>
/*Process entries*/


#define PROCESS_STATUS_RUNNING 1
#define PROCESS_STATUS_WAITING 2
#define PROCESS_STATUS_YIELDING 3

/*Forward decl*/
struct address_space;

typedef struct context {
  isr_regs registers;
  struct address_space * space;
  int pid;
  int status;
  struct context * next;
} context_t;


void initialize_scheduler();
void schedule(isr_regs *);
int create_process();
int execve_elf (context_t *, void *, unsigned long, const char *);
int fork (isr_regs *);
context_t * get_process(int);
void begin_schedule (isr_regs *);
int get_current_process();
void set_current_process (int);
#endif
