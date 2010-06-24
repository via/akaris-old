#ifndef I386_SYSCALL_H
#define I386_SYSCALL_H

#include <i386/interrupt.h>
#include <i386/kfifo.h>

typedef enum {
  SYSCALL_KDEBUG,
  FIFO_PIPE,
  FIFO_OP,
  LINK_IRQ,
  REQUEST_IO,
} syscall_number;

/*For ALL syscalls, EAX defines the function, EDX defines parameter*/

typedef struct fifo_op {
  uint32 fifo_id;
  void * buf;
  uint32 length;
  enum {
    FIFO_OP_READ,
    FIFO_OP_WRITE,
    FIFO_OP_BLOCK,
  } operation;
  uint8 num_waits;
  uint32 waitlist[16];
  kfifo_error err;
} fifo_op_t;



void enable_syscall();
void syscall_handler(isr_regs *);


#endif
