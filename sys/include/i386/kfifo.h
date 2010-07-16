#ifndef I386_KFIFO_H
#define I386_KFIFO_H

#include <i386/types.h>
#include <mutex.h>
#include <i386/context.h>
#include <i386/kqueue.h>

typedef struct kfifo_acl_entry {
  uint32 pid;
  struct kfifo_acl_entry *next;
} kfifo_acl_entry_t;

static const uint32 kfifo_kernel_pid = 0xFFFFFFFF;

typedef struct kfifo {
  kfifo_acl_entry_t *recvlist;
  kfifo_acl_entry_t *sendlist;
  uint32 size; /*BYTES*/
  uint32 start;
  uint32 end;
  mutex_t lock;
  uint32 fifo_id;
  struct kfifo * next;
  struct kevent * ke_list;
} kfifo_t;

typedef enum {
  KFIFO_SUCCESS = 0,
  KFIFO_ERR_LOCKED,
  KFIFO_ERR_MEM,
  KFIFO_ERR_PERM,
  KFIFO_ERR_FULL,
  KFIFO_ERR_BUFLEN,
  KFIFO_ERR_EXIST,
} kfifo_error;

void kfifo_init ();
kfifo_error kfifo_create_fifo (uint32 *fifo_id, uint32 *recv, uint32 *send, uint32 size);
kfifo_error kfifo_write_fifo (uint32 fifo_id, uint32 mypid, const void * buf, uint32 len);
kfifo_error kfifo_read_fifo  (uint32 fifo_id, uint32 mypid, void * buf, uint32 len);
kfifo_error kfifo_close_fifo (uint32 fifo_id, uint32 mypid);
kfifo_error kfifo_clone_fifo (uint32 fifo_id, uint32 newpid);
kfifo_error kfifo_update_senders (uint32 oldpid, uint32 newpid);

int kfifo_add_kqueue_event (struct kevent *ke);
int kfifo_delete_kqueue_event (struct kevent *ke);

#endif

