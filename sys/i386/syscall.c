/*syscall.c*/

#include <i386/interrupt.h>
#include <i386/bootvideo.h>
#include <i386/context.h>
#include <i386/process.h>
#include <i386/syscall.h>
#include <i386/mailbox.h>
#include <i386/pic.h>

void enable_syscall() {

  link_irq(0x80, &syscall_handler);

  bootvideo_printf("System Call functionality enabled\n");

}


void syscall_handler(isr_regs * regs) {
  
  context_t * c = get_process (get_current_process ());
  mailbox_t *mb = c->mailboxes;

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
    regs->edx = send_message ((message_t *)regs->edx, 0);
    break;

  case SYSCALL_FUNCTION_MAILBOX_RECEIVE:
    regs->edx = (int) next_message ((mailbox_t *)regs->edx);
    break;
  case SYSCALL_FUNCTION_BLOCK:

    while (mb != 0) {
      if (mb->first == (mb->last + 1) % mb->size) {
	mb = mb->next;
	continue;
      }
      return;
    }
    c->status = PROCESS_STATUS_WAITING;
    schedule (regs);
    break;
  case SYSCALL_LINK_IRQ:
    link_irq_to_pid (regs->edx, c->pid);
    break;
  case SYSCALL_IO:
    if (regs->edx & 0xFF000000) { /*High byte > 1 == Input */ 
      regs->edx = inportb (0xFFFF & regs->edx);
    } else {
      outportb (0xFFFF & regs->edx, (regs->edx & 0xFF0000) >> 16);
    }
    break;
  default:
    bootvideo_printf("Undefined Syscall requested!\n");
  }

}
