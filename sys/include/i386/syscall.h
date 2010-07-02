#ifndef I386_SYSCALL_H
#define I386_SYSCALL_H

#include <i386/interrupt.h>
#include <i386/kfifo.h>
#include <i386/device_interface.h>

typedef enum {
  SYSCALL_KDEBUG,
  FIFO_PIPE,
  FORK,
  FIFO_OP,
  DEVNODE_OP,
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
    FIFO_OP_CLOSE,
  } operation;
  uint8 num_waits;
  uint32 waitlist[16];
  kfifo_error err;
} fifo_op_t;

typedef struct devnode_op {
  enum {
    DEVNODE_OP_REGISTER,
    DEVNODE_OP_CONNECT,
    DEVNODE_OP_ACCEPT,
  } operation;
  char * devname;
  uint32 fifos[2];
  dev_error err;
} devnode_op_t;



void enable_syscall();
void syscall_handler(isr_regs *);


#endif
