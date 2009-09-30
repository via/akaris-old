#include <i386/process.h>

slab_entry * context_slab;
context_t * context_list;

int next_avail_pid;
context_t * cur_process;

void initialize_scheduler() {
  context_slab = create_slab(sizeof(context_t));
  context_list = (context_t *) 0;
  next_avail_pid = 1;
}

int create_process(int addr, int length) {
  context_t * new_context, *t;
  new_context = (context_t*)allocate_from_slab(context_slab);
  if (context_list == 0) {
    context_list = new_context;
  } else {
    for (t = context_list;
	 t->next != 0;
	 t = t->next); /*Get to end*/
    t->next = new_context;
  }
  new_context->registers.eax = 0xDEADBEEF;
  new_context->registers.gs = 0x23;
  new_context->registers.fs = 0x23;
  new_context->registers.es = 0x23;
  new_context->registers.ds = 0x23;
  new_context->registers.ss = 0x23;
  new_context->registers.eip = 0x40000000;
  new_context->registers.cs = 0x1B;
  new_context->registers.eflags = 0x200; /*IF set*/
  new_context->registers.useresp = 0xC0000FFF; /*0xC0000FFF;*/

  new_context->space = create_address_space();
  expand_region(new_context->space->core, length / 4096 + 1);
  expand_region(new_context->space->stack, 1);
  set_cr3(new_context->space->cr3);
  
  char * s, * d;
  for (d = (char *)0x40000000, s = (char*)addr;
       (int)s < addr + length;
       s++, d++) {
    *d = *s;
  }
  
  new_context->pid = next_avail_pid;
  next_avail_pid++;

  new_context->status = PROCESS_STATUS_YIELDING;

  return new_context->pid;
}

context_t * get_process(int pid) {
  if (context_list == 0)
    return (context_t*) 0;

  context_t * c;
  for (c = context_list;
       (c->pid != pid);
       c = c->next);
  bootvideo_printf("Returning pid %d\n", c->pid);
  cur_process = c;
  return c;
  
}
  
void schedule(isr_regs * regs) {
  memcpy((char*)&cur_process->registers, (char*)regs, sizeof(isr_regs));
  cur_process = cur_process->next;
  if (cur_process == 0)
    cur_process = context_list;
  memcpy((char*)regs,(char*) &cur_process->registers, sizeof(isr_regs));
  
  set_cr3(cur_process->space->cr3);
}

int get_current_process() {
  return cur_process->pid;
}
