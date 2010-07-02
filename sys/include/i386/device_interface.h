#ifndef I386_DEVICE_INTERFACE_H
#define I386_DEVICE_INTERFACE_H

#include <i386/types.h>
#include <i386/kfifo.h>

typedef struct devnode {
  uint32 pid;
  char devname[64];
  uint32 accept_fifo_id;
  struct devnode *next;
} devnode_t;

typedef enum {
  DEV_SUCCESS = 0,
  DEV_ERR_PERM,
  DEV_ERR_LOCKED,
  DEV_ERR_EXIST,
} dev_error;

void devnode_init ();
dev_error devnode_register (const char * devname, uint32 mypid);
dev_error devnode_connect (const char * devname, uint32 * pipes, uint32 mypid);
dev_error devnode_accept (const char * devname, uint32 *fifo, uint32 mypid);
dev_error devnode_unregister (const char * devname, uint32 mypid);

#endif

