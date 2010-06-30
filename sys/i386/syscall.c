/*syscall.c*/
#include <i386/types.h>
#include <i386/interrupt.h>
#include <i386/bootvideo.h>
#include <i386/context.h>
#include <i386/process.h>
#include <i386/syscall.h>
#include <i386/pic.h>
#include <i386/kfifo.h>
void enable_syscall() {

  link_irq(0x80, &syscall_handler);

  bootvideo_printf("System Call functionality enabled\n");

}


void syscall_handler(isr_regs * regs) {
  uint32 sends[] = {1, 0};
  uint32 receives[] = {1, 0};
  uint32 fifos[2];   
  fifo_op_t *fop;
/*  context_t * c = get_process (get_current_process ());*/

  switch (regs->eax) {
  case SYSCALL_KDEBUG:
    bootvideo_printf("%s", regs->edx);
    break;
  case REQUEST_IO:
    if (regs->edx & 0xFF000000) { /*High byte > 1 == Input */ 
      regs->edx = inportb (0xFFFF & regs->edx);
    } else {
      outportb (0xFFFF & regs->edx, (regs->edx & 0xFF0000) >> 16);
    }
    break;
  case FORK:
    regs->edx = fork (regs);
    break;
  case FIFO_PIPE:
    receives[0] = get_current_process ();
    sends[0] = get_current_process ();
    kfifo_create_fifo (&fifos[0], receives, sends, 4096);
    kfifo_create_fifo (&fifos[1], sends, receives, 4096);
    regs->ecx = fifos[0];
    regs->edx = fifos[1];
    break;
  case FIFO_OP:
    fop = (fifo_op_t *)regs->edx;
    switch (fop->operation) {
      case FIFO_OP_WRITE:
        fop->err = kfifo_write_fifo (fop->fifo_id, get_current_process (), fop->buf, fop->length);
        break;
      case FIFO_OP_READ:
        fop->err = kfifo_read_fifo (fop->fifo_id, get_current_process (), fop->buf, fop->length);
        break;
      case FIFO_OP_BLOCK:
        break;
      case FIFO_OP_CLOSE:
        fop->err = kfifo_close_fifo (fop->fifo_id, get_current_process ());
    }
    break;
  default:
    bootvideo_printf("Undefined Syscall requested!\n");
  }

}
