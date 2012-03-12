#ifndef I386_SYSCALL_H
#define I386_SYSCALL_H

#include <i386/interrupt.h>
#include <common/kfifo.h>
#include <common/device_interface.h>
#include <common/kqueue.h>

typedef enum {
  SYSCALL_KDEBUG,
  FIFO_PIPE,
  FORK,
  FIFO_OP,
  DEVNODE_OP,
  KQUEUE_OP,
  LINK_IRQ,
  REQUEST_IO,
  REQUEST_MMAP,
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
    DEVNODE_OP_LINK_IRQ,
  } operation;
  char * devname;
  uint8 irq;
  uint32 fifos[2];
  dev_error err;
} devnode_op_t;

/*
typedef struct kqueue_event {
  kevent_filter_t filter;
  kevent_flag_t flag;
  uint32 ident;
} kqueue_event_t;
*/

typedef struct kqueue_op {
  enum {
    KQUEUE_OP_CREATE,
    KQUEUE_OP_EVENT,
    KQUEUE_OP_POLL,
    KQUEUE_OP_BLOCK,
  } operation;
  uint32 kqueue_id;
  kqueue_error err;
  struct kevent newevent;
  struct kevent *changedevents;
  uint32 max_events;
} kqueue_op_t;

typedef struct mmap_op {
  enum {
    MMAP_OP_ANON,
    MMAP_OP_DIRECTED,
    MMAP_OP_FIFO,
  } operation;
  unsigned long virt;
  unsigned long phys;
  unsigned long size;
  uint32 fifo;
} mmap_op_t;

void enable_syscall();
void syscall_handler(isr_regs *);


#endif
