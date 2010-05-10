/*syscall.c*/

#include <i386/syscall.h>

void enable_syscall() {

  link_irq(0x80, &syscall_handler);

  bootvideo_printf("System Call functionality enabled\n");

}


void syscall_handler(isr_regs * regs) {
  

  switch (regs->eax) {
  case SYSCALL_FUNCTION_KDEBUG:
    bootvideo_printf("%s", regs->edx);
    break;
  case SYSCALL_FUNCTION_MMAP_LIST:
    context_print_mmap ();
    break;

  default:
    bootvideo_printf("Undefined Syscall requested!\n");
  }

}
