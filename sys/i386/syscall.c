/*syscall.c*/

#include <i386/syscall.h>
#include <i386/mailbox.h>
void enable_syscall() {

  link_irq(0x80, &syscall_handler);

  bootvideo_printf("System Call functionality enabled\n");

}


void syscall_handler(isr_regs * regs) {
  
  context_t * c = get_process (get_current_process ());

  switch (regs->eax) {
  case SYSCALL_FUNCTION_KDEBUG:
    bootvideo_printf("%s", regs->edx);
    break;
  case SYSCALL_FUNCTION_MMAP_LIST:
    context_print_mmap ();
    break;
  case SYSCALL_FUNCTION_MAILBOX_CREATE:
    regs->edx = (int) create_mailbox (c, regs->edx, regs->ecx);
    break;
  case SYSCALL_FUNCTION_MAILBOX_SEND:
    regs->edx = send_message (regs->ecx, (message *)regs->edx);
    break;

  case SYSCALL_FUNCTION_MAILBOX_RECEIVE:
    regs->edx = (int) next_message ((mailbox *)regs->edx);
    break;
  case SYSCALL_FUNCTION_BLOCK:
    c->status = PROCESS_STATUS_WAITING;
      schedule (regs);
    break;

  default:
    bootvideo_printf("Undefined Syscall requested!\n");
  }

}
