/*! \brief Manage the device tree, as well as allow programs to access drivers.
 *
 */

#include <config.h>
#include <mutex.h>
#include <i386/types.h>
#include <i386/bootvideo.h>
#include <i386/slab.h>
#include <i386/kfifo.h>
#include <i386/device_interface.h>

static mutex_t devlock;
devnode_t * devlist;

slab_entry_t * devnode_slab;

void
devnode_init () {
  devnode_slab = create_slab (sizeof (devnode_t));
}


dev_error
devnode_register (const char * devname, uint32 mypid) {
  
  if (test_and_set (1, &devlock)) {
    return DEV_ERR_LOCKED;
  }

  devnode_t *temp = devlist;
  while (temp != NULL) {
    if (strncmp (temp->devname, devname, 64) == 0) {
      /*Device name already exists*/
      test_and_set (0, &devlock);
      return DEV_ERR_EXIST;
    }
    temp = temp->next;
  }

  devnode_t * newdev = allocate_from_slab (devnode_slab);
  newdev->pid = mypid;
  newdev->next = devlist;
  memcpy (newdev->devname, devname, strlen (devname, 64));
  devlist = newdev;

  test_and_set (0, &devlock);

  return DEV_SUCCESS;
}


dev_error
devnode_connect (const char * devname, uint32 * rpipes, uint32 mypid) {
                                 
  uint32 pipes[2];
  uint32 senders[2];
  uint32 receivers[2];

  if (test_and_set (1, &devlock)) {
    return DEV_ERR_LOCKED;
  }                              
  devnode_t *temp = devlist;
  while (temp != NULL) {
    if (strncmp (temp->devname, devname, 64) == 0) break;
    temp = temp->next;
  }  

  if (temp == NULL) {
    test_and_set (0, &devlock);
    return DEV_ERR_LOCKED;
  }
  receivers[0] = temp->pid;
  senders[0] = mypid;

  kfifo_create_fifo (&pipes[0], receivers, senders, PAGE_SIZE);
  kfifo_create_fifo (&pipes[1], senders, receivers, PAGE_SIZE);
  kfifo_write_fifo (temp->accept_fifo_id, kfifo_kernel_pid, pipes, sizeof (uint32) * 2);

  rpipes[0] = pipes[0];
  rpipes[1] = pipes[1];

  test_and_set (0, &devlock);

  return DEV_SUCCESS;
}

dev_error
devnode_accept (const char * devname, uint32 *fifo, uint32 mypid) {

  devnode_t *temp = devlist;
  while (temp != NULL) {
    if (strncmp (temp->devname, devname, 64) == 0) break;
    temp = temp->next;
  }  

  if (temp == NULL) {
    return DEV_ERR_EXIST;
  }

  uint32 recvlist[] = {0, 0};
  uint32 sendlist[] = {kfifo_kernel_pid, 0};
  recvlist[0] = mypid;
  kfifo_create_fifo (fifo, recvlist, sendlist, PAGE_SIZE);
  temp->accept_fifo_id = *fifo;
  
  return DEV_SUCCESS;
}

