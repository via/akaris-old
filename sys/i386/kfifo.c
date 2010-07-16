/* kfifo.c -- handles the creation, reading/writing, and cloning of kernel
 * fifos.  This is the new main method of IPC in akaris. */
#include <mutex.h>
#include <config.h>
#include <i386/types.h>
#include <i386/slab.h>
#include <i386/paging.h>
#include <i386/context.h>
#include <i386/process.h>
#include <i386/bootvideo.h>
#include <i386/kqueue.h>
#include <i386/kfifo.h>


static mutex_t fifo_list_mutex = 0;
static uint32 cur_fifo_id = 1;
static kfifo_t * fifo_list;
static slab_entry_t * fifo_slab;
static slab_entry_t * acl_slab;

/*! \brief Initializes the fifo subsystem.
 *
 * Must be called before any fifo's are created or used.
 */
void kfifo_init () {
  
  fifo_slab = create_slab (sizeof (kfifo_t));
  acl_slab = create_slab (sizeof (kfifo_acl_entry_t));

  fifo_list = NULL;
}

/*! \brief Creates a fifo.
 *
 * Creates a fifo with a given list of pids that can receive from the fifo, sent
 * to the fifo, and the size of the fifo to be created
 *
 * \param fifo_id The created fifo will have its ID stored in this parameter
 *
 * \param recv Null terminated array of pids that can read from the fifo
 *
 * \param send Null terminated array of pids that can send to the fifo.
 *
 * \param size Size in bytes of Fifo.  Will be rounded up to nearest pagesize.
 *
 * \returns error code, 0 on success
 */
kfifo_error kfifo_create_fifo (uint32 *fifo_id, uint32 *recv, uint32 *send, uint32 size) {

  int cur;
  kfifo_acl_entry_t *newentry;

  if (test_and_set (1, &fifo_list_mutex) ) {
    return KFIFO_ERR_LOCKED;
  }

  kfifo_t * newfifo = (kfifo_t *) allocate_from_slab (fifo_slab);

  newfifo->fifo_id = cur_fifo_id++;
  *fifo_id = newfifo->fifo_id;
  newfifo->size = (((size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE);
  newfifo->start = 0;
  newfifo->end = 0;
  newfifo->lock = 0;
  newfifo->ke_list = NULL;
  
  newfifo->recvlist = (kfifo_acl_entry_t *) allocate_from_slab (acl_slab);
  newentry = newfifo->recvlist;
  /*Process the receive list*/
  for (cur = 0; recv[cur] != 0; ++cur) {
    newentry->pid = recv[cur];
    if (recv[cur + 1] != 0) {
      newentry->next = (kfifo_acl_entry_t *) allocate_from_slab (acl_slab);
      newentry = newentry->next;
    }
  }

  /*Process the send list*/
   
  newfifo->sendlist = (kfifo_acl_entry_t *) allocate_from_slab (acl_slab);
  newentry = newfifo->sendlist;
  /*Process the receive list*/
  for (cur = 0; send[cur] != 0; ++cur) {
    newentry->pid = send[cur];
    if (send[cur + 1] != 0) {
      newentry->next = (kfifo_acl_entry_t *) allocate_from_slab (acl_slab);
      newentry = newentry->next;
    }
  }
  
  /*Create memory regions in all the receive address spaces*/
  memory_region_t * mr = create_region (get_process (recv[0])->space, 0, 
      (size + PAGE_SIZE ) / PAGE_SIZE, MR_TYPE_IPC, MR_ATTR_RO, newfifo->fifo_id);
  map_user_region_to_physical (mr, 0);

  if (fifo_list == NULL) {
    fifo_list = newfifo;
  } else {
    newfifo->next = fifo_list;
    fifo_list = newfifo;
  }

  /*Iterate over other receive addres spaces and clone the region into it*/
  test_and_set (0, &fifo_list_mutex);
  return KFIFO_SUCCESS;
}

/*! \brief Write into a FIFO
 *
 * Checks permissions to make sure write access is possible, and then attempts
 * to write data into the fifo.  The buffer must be located in a valid CORE
 * region in the process.  If the data length is greater than the buffer is
 * capable of holding, writing will fail with KFIFO_ERR_BUFLEN.  If there is
 * simlpy not enough room in the buffer at the time, KFIFO_ERR_FULL will be
 * generated
 *
 * \param fifo_id FIFO handle
 *
 * \param mypid Current process PID
 *
 * \param buf Pointer to the buffer in the current context to send.
 *
 * \param len Size (in bytes) of buffer to be sent.
 *
 * \returns Error Code
 */
kfifo_error
kfifo_write_fifo (uint32 fifo_id, uint32 mypid, const void * buf, uint32 len) {

  if (test_and_set (1, &fifo_list_mutex) ) {
    return KFIFO_ERR_LOCKED;
  }

  /* Find the correct fifo*/
  kfifo_t *fifo = fifo_list;
  while (fifo != NULL) {
    if (fifo->fifo_id == fifo_id) break;
    fifo = fifo->next;
  }
  if (fifo == NULL) {
    test_and_set (0, &fifo_list_mutex);
    return KFIFO_ERR_EXIST;
  }

  test_and_set (0, &fifo_list_mutex);

  if (test_and_set (1, &fifo->lock)) {
    return KFIFO_ERR_LOCKED;
  }

  if (fifo->size - 1 < len) {
    test_and_set (0, &fifo->lock);
    return KFIFO_ERR_BUFLEN;
  }

  int curfree = (fifo->start - fifo->end);
  if (curfree < 0) {
    curfree += fifo->size;
  }                      
  curfree = fifo->size - curfree - 1;

  if ((unsigned) curfree < len) {
    test_and_set (0, &fifo->lock);
    return KFIFO_ERR_FULL;
  }

  /*Check for write permission */
  kfifo_acl_entry_t * acl = fifo->sendlist;
  while (acl != NULL) {
    if (acl->pid == mypid) break;
    acl = acl->next;
  }
  if (acl == NULL) {
    test_and_set (0, &fifo->lock);
    return KFIFO_ERR_PERM;
  }

  /*Find memory region in a receive address space that is for this fifo*/
  memory_region_t *r_mr = get_process(fifo->recvlist->pid)->space->first->next;
  while (r_mr->type != MR_TYPE_SENTINEL) {
    if ( (r_mr->type == MR_TYPE_IPC) && (r_mr->parameter == fifo->fifo_id)) break;
    r_mr = r_mr->next;
  }
  if (r_mr->type == MR_TYPE_SENTINEL) {
    bootvideo_printf ("Oh god, something went awfully wrong!\n");
    while (1);
  }
  
  /* This seems kind of hacky, consider changing it.
   * We just need to make a copy of the receivers region for the fifo in our
   * local address space */
  memory_region_t * mr;
  if (mypid != kfifo_kernel_pid) {
    mr = clone_region (get_process(mypid)->space, r_mr, 0);
  } else {
    mr = clone_region (get_process (get_current_process ())->space, r_mr, 0);
  }

  /*Write the buffer into the fifo ring buffer*/
  if (fifo->size - fifo->start >= len ) {
    memcpy ((void *)mr->virtual_address + fifo->start, buf, len);            
  } else {
    if (fifo->start + len > fifo->size) {
      memcpy ((void *)mr->virtual_address + fifo->start, buf, fifo->size - fifo->start);
      memcpy ((void *)mr->virtual_address, buf + (fifo->size - fifo->start), len - (fifo->size - fifo->start)); 
    } else {
      memcpy ((void *)mr->virtual_address + fifo->start, buf, len);                                             
    }
  }

  delete_region (mr); 

  fifo->start += len;
  if (fifo->start >= fifo->size) {
    fifo->start -= fifo->size;
  }

  test_and_set (0, &fifo->lock);

  /*Trigger any events*/
  kevent_t * ke = fifo->ke_list;
  while (ke != NULL) {
    kqueue_trigger_event (ke);
    ke = ke->next;
  }

  return KFIFO_SUCCESS;
}

/*! \brief Read from a kernel fifo
 *
 * Attempts to read len bytes from fifo fifo_id into buf.  Process with pid
 * mypid must have read access to the fifo. Will return KFIFO_ERR_BUFLEN in the
 * case that the number of requested bytes is not available.  Will only return
 * successfully when the full number of bytes can be read into the buffer*
 *
 * \param fifo_id Kernel FIFO from which to read
 *
 * \param mpid Current process's PID
 *
 * \param buf Buffer (in a userspace CORE region) to write into from FIFO
 *
 * \param len number of bytes to read from the fifo
 */
kfifo_error
kfifo_read_fifo (uint32 fifo_id, uint32 mypid, void * buf, uint32 len) {
 
  if (test_and_set (1, &fifo_list_mutex)) {
    return KFIFO_ERR_LOCKED;
  }
  kfifo_t *fifo = fifo_list;
  while (fifo != NULL) {
    if (fifo->fifo_id == fifo_id) break;
    fifo = fifo->next;
  }
  if (fifo == NULL) {
    test_and_set (0, &fifo_list_mutex);
    return KFIFO_ERR_EXIST;
  }
  
  if (test_and_set (1, &fifo->lock)) {
    test_and_set (0, &fifo_list_mutex);
    return KFIFO_ERR_LOCKED;
  }
  test_and_set (0, &fifo_list_mutex);

  /*Determine if this pid can read from this fifo*/
  kfifo_acl_entry_t *cur = fifo->recvlist;
  while (cur != NULL) {
    if (cur->pid == mypid) break;
    cur = cur->next;
  }
  if (cur == NULL) {
    test_and_set (0, &fifo->lock);
    return KFIFO_ERR_PERM;
  }

  int cursize = fifo->end - fifo->start;
  if (cursize < 0) {
    cursize += fifo->size;
  }

  if ((unsigned)cursize < len) {
    test_and_set (0, &fifo->lock);
    return KFIFO_ERR_BUFLEN;
  }

  /*Find the memory region for this fifo*/
  memory_region_t *mr = get_process(mypid)->space->first->next;
  while (mr->type != MR_TYPE_SENTINEL) {
    if ( (mr->type == MR_TYPE_IPC) && (mr->parameter == fifo_id)) break;
    mr = mr->next;
  }
  if (mr->type == MR_TYPE_SENTINEL) {
    bootvideo_printf ("Jesus, wtf happened?\n");
    while (1);
  }



  if ( fifo->size - fifo->end >= len) {
    memcpy (buf, (void *)( mr->virtual_address + fifo->end), len);  
  } else {
    memcpy (buf, (void *)( mr->virtual_address + fifo->end), fifo->size - fifo->end);                    
    memcpy (buf + (fifo->size - fifo->end), (void *) mr->virtual_address, len - (fifo->size - fifo->end)); 
  }

  fifo->end += len;
  if (fifo->end >= fifo->size) {
    fifo->end -= fifo->size;
  }
  
  test_and_set (0, &fifo->lock);

  return KFIFO_SUCCESS;
}

/*! \brief Make the fifo register this pid as a valid receiver
 * \param fifo_id Fifo being cloned.
 * \param newpid Process that will get receive access
 * \returns fifo error
 */
kfifo_error kfifo_clone_fifo (uint32 fifo_id, uint32 newpid) {
  
  if (test_and_set (1, &fifo_list_mutex)) {
    return KFIFO_ERR_LOCKED;
  }
  kfifo_t *fifo = fifo_list;
  while (fifo != NULL) {
    if (fifo->fifo_id == fifo_id) break;
    fifo = fifo->next;
  }
  if (fifo == NULL) {
    test_and_set (0, &fifo_list_mutex);
    return KFIFO_ERR_EXIST;
  }
  
  if (test_and_set (1, &fifo->lock)) {
    test_and_set (0, &fifo_list_mutex);
    return KFIFO_ERR_LOCKED;
  }
  test_and_set (0, &fifo_list_mutex);

  kfifo_acl_entry_t *newacl = allocate_from_slab (acl_slab);
  newacl->pid = newpid;
  newacl->next = fifo->recvlist;
  fifo->recvlist = newacl;

  test_and_set (0, &fifo->lock);

  return KFIFO_SUCCESS;
}
 
/*! \brief Go through all fifos and update write accesses
 *
 * There is probably a better way of doing this.  This goes through
 * entries in the fifo list, and whenever it finds oldpid as a writer
 * it will add newpid to it as well. Its slow.
 *
 * \param oldpic pid of original process
 * \param newpid pid of new process
 * \returns error
 */
kfifo_error kfifo_update_senders (uint32 oldpid, uint32 newpid) {

  if (test_and_set (1, &fifo_list_mutex)) {
    return KFIFO_ERR_LOCKED;
  }  

  
  /*Iterate over all fifos*/
  kfifo_t *fifo = fifo_list;
  while (fifo != NULL) {
    kfifo_acl_entry_t *newacl = allocate_from_slab (acl_slab);
    newacl->pid = newpid;
   
    /*Iterate over all the senders*/
    kfifo_acl_entry_t *curacl = fifo->sendlist;
    while (curacl != NULL) {
      if (curacl->pid == oldpid) {
        newacl->next = curacl->next;
        curacl->next = newacl;
        curacl = newacl;
      }
      curacl = curacl->next;
    }

    fifo = fifo->next;
  }
  test_and_set (0, &fifo_list_mutex);
  return KFIFO_SUCCESS;
}

/*! \brief Close a fifo from the point of view of the process.
 *
 * If the last sender or last receiver is deleted, the fifo is deleted.
 *
 * \param fifo_id Fifo to close.
 * \param mypid Pid that is requesting the close
 * \returns Error code
 */
kfifo_error kfifo_close_fifo (uint32 fifo_id, uint32 mypid) {

  /*If its just a write fifo, remove our pid from the sender list
   *If its a read fifo, delete the memory region and remove the pid.
   */
  if (test_and_set (1, &fifo_list_mutex)) {
    return KFIFO_ERR_LOCKED;
  }
  kfifo_t *fifo = fifo_list;
  while (fifo != NULL) {
    if (fifo->fifo_id == fifo_id) break;
    fifo = fifo->next;
  } 
  if (test_and_set (1, &fifo->lock)) {
    test_and_set (0, &fifo_list_mutex);
    return KFIFO_ERR_LOCKED;
  }

  /*See if this pid is in this fifo's sender list*/
  kfifo_acl_entry_t *curacl = fifo->sendlist;
  while (curacl != NULL) {
    if (curacl->pid == mypid) break;
    curacl = curacl->next;
  }
  if (curacl != NULL) {
    if (fifo->sendlist == curacl) { /*First entry*/
      fifo->sendlist = fifo->sendlist->next;
    } else {
      kfifo_acl_entry_t *temp = fifo->sendlist;
      while (temp->next != curacl) temp = temp->next; /*Get to entry prior to the current pid*/
      temp->next = curacl->next;
    }
    deallocate_from_slab (acl_slab, curacl);
  }

  /*See if the pid is in the receive list*/
  curacl = fifo->recvlist;
  while (curacl != NULL) {
    if (curacl->pid == mypid) break;
    curacl = curacl->next;
  }
  if (curacl != NULL) {
    if (fifo->recvlist == curacl) { /*First entry in list*/
      fifo->recvlist = fifo->recvlist->next;
    } else {
      kfifo_acl_entry_t *temp = fifo->recvlist;
      while (temp->next != curacl) temp = temp->next;
      temp->next = curacl->next;
    }
  }
  
  memory_region_t *r_mr = get_process(mypid)->space->first->next;
  while (r_mr->type != MR_TYPE_SENTINEL) {
    if ( (r_mr->type == MR_TYPE_IPC) && (r_mr->parameter == fifo->fifo_id)) break;
    r_mr = r_mr->next;
  }  
  if (r_mr->type != MR_TYPE_SENTINEL) {
    delete_region (r_mr);
  }
  deallocate_from_slab (acl_slab, curacl);

  if ( (fifo->recvlist == NULL ) || (fifo->sendlist == NULL)) {
    /*One of the two lists is empty -- we should delete it*/
    
    /*Eliminate everything in the receive list*/
    curacl = fifo->recvlist;
    while (curacl != NULL) {
      kfifo_acl_entry_t * temp = curacl->next;
      memory_region_t *r_mr = get_process(mypid)->space->first->next;
      while (r_mr->type != MR_TYPE_SENTINEL) {
        if ( (r_mr->type == MR_TYPE_IPC) && (r_mr->parameter == fifo->fifo_id)) break;
        r_mr = r_mr->next;
      } 
      delete_region (r_mr);
      deallocate_from_slab (acl_slab, curacl);
      curacl = temp;
    }

    /*Delete all the sender list*/
    curacl = fifo->recvlist;
    while (curacl != NULL) {
      kfifo_acl_entry_t * temp = curacl->next;
      deallocate_from_slab (acl_slab, curacl);
      curacl = temp;
    }

    if (fifo_list == fifo) { /*First entry */
      kfifo_t * temp = fifo_list->next;
      deallocate_from_slab (fifo_slab, fifo_list);
      fifo_list = temp;
    } else {
      kfifo_t * temp = fifo_list;
      while (temp->next != fifo) temp = temp->next;
      temp->next = temp->next->next;
      deallocate_from_slab (fifo_slab, fifo);
    }

  } else {
    test_and_set (0, &fifo->lock);  
  }

  test_and_set (0, &fifo_list_mutex);

  return KFIFO_SUCCESS;
}

int
kfifo_add_kqueue_event (struct kevent *ke) {

  /*Add to the linked list for the fifo*/

  kfifo_t *curfifo = fifo_list;
  while (curfifo != NULL) {
    if (curfifo->fifo_id == ke->ident) break;
    curfifo = curfifo->next;
  }
  if (curfifo == NULL) {
    return 1;
  }
  
  ke->hook_next = curfifo->ke_list;
  curfifo->ke_list = ke;

  /*Check to see if its already triggered*/
  switch (ke->flag) {
    case KEVENT_FLAG_FIFO_READABLE: 
      if (curfifo->end != curfifo->start) {
        test_and_set (1, &ke->triggered);
      }
      break;
    case KEVENT_FLAG_FIFO_WRITABLE:
      /*Not implemented*/
      break;
    default:
      break;
  }

  return 0;
}

int kfifo_delete_kqueue_event (kevent_t *ke) {

  /*TODO: do*/
  ke->triggered = 0;

  return 0;
}

