#include <i386/process.h>
#include <i386/mailbox.h>
#include <i386/gdt.h>


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
  new_context->next = 0;
  new_context->mailboxes = 0;
  new_context->registers.eax = 0xDEADBEEF;
  new_context->registers.gs = 0x23;
  new_context->registers.fs = 0x23;
  new_context->registers.es = 0x23;
  new_context->registers.ds = 0x23;
  new_context->registers.ss = 0x23;
  new_context->registers.eip = 0x40000000;
  new_context->registers.cs = 0x1B;
  new_context->registers.eflags = 0x200; /*IF set*/
  new_context->registers.useresp = 0xBFFFFFF0; /*0xC0000FFF;*/

  new_context->space = create_address_space();
  expand_region(new_context->space->core, length / 4096 + 1);
  expand_region(new_context->space->stack, -1);
  set_cr3(new_context->space->cr3);

  char * s, * d;
  for (d = (char *)0x40000000, s = (char*)addr;
       (int)s < addr + length;
       s++, d++) {
    *d = *s;
  }
  
  new_context->pid = next_avail_pid;
  next_avail_pid++;

  new_context->status = PROCESS_STATUS_RUNNING;

  return new_context->pid;
}

context_t * get_process(int pid) {
  if (context_list == 0)
    return (context_t*) 0;

  context_t * c;
  for (c = context_list;
       (c->pid != pid) && (c->next != 0);
       c = c->next);
  if (c->pid != pid) {
    bootvideo_printf("Invalid pid %d\n", pid);
    return 0;
  } else {
    return c;
  }
}



void schedule(isr_regs * regs) {

  int temp_stack[100];

  if (kernel_reenter) {
    return;
  }
  memcpy((char*)&cur_process->registers, (char*)regs, sizeof(isr_regs));

  context_t *orig = cur_process;

  do {
    cur_process = cur_process->next;
    if (cur_process == 0)
      cur_process = context_list;
    if (cur_process == orig) { /*We've looped the whole list*/
      if (cur_process->status != PROCESS_STATUS_RUNNING) {
	kernel_reenter = 1;
	set_kernel_tss_stack ((void*)temp_stack);
	__asm__("sti\n"
		"hlt");
	set_kernel_tss_stack (0);
	__asm__("cli");
	kernel_reenter = 0;

      }
    }
  } while (cur_process->status != PROCESS_STATUS_RUNNING);
  memcpy((char*)regs,(char*) &cur_process->registers, sizeof(isr_regs));
  
  set_cr3(cur_process->space->cr3);
}

int get_current_process() {
  return cur_process->pid;
}

void set_current_process (int pid) {
  cur_process = get_process ( pid );
}
