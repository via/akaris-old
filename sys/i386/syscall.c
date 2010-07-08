/*syscall.c*/
#include <config.h>
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
  kfifo_error e;
  devnode_op_t *dop;
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
  case DEVNODE_OP:
    dop = (devnode_op_t *)regs->edx;
    switch (dop->operation) {
      case DEVNODE_OP_REGISTER:
        dop->err = devnode_register (dop->devname, get_current_process ());
        break; 
      case DEVNODE_OP_CONNECT:
        dop->err = devnode_connect (dop->devname, dop->fifos, get_current_process ());
        break;
      case DEVNODE_OP_ACCEPT:
        dop->err = devnode_accept (dop->devname, dop->fifos, get_current_process ());
        break;
      case DEVNODE_OP_LINK_IRQ:
        receives[0] = get_current_process ();
        sends[0] = kfifo_kernel_pid;
        e = kfifo_create_fifo (dop->fifos, receives, sends, PAGE_SIZE);
        link_irq_to_fifo (dop->irq, dop->fifos[0]);
        break;
    }
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
    if (fop->err == KFIFO_ERR_LOCKED) {
      bootvideo_printf ("Locked when it shouldn't be!\n");
    }
    break;
  default:
    bootvideo_printf("Undefined Syscall requested!\n");
  }

}
