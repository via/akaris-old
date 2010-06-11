/* 
 * mailbox.c
 * This has all functions related to the creation and use
 * of mailboxes.  A process can create a mailbox that
 * can allow any pid to send to (-1) or a specific PID.
 * It can then call next_message to receive any waiting
 * message (and will receive a null if none are ready),
 * or send_message to send a message to another PID.
 * This will return zero on failure, one on success*/

#include <i386/mailbox.h>

slab_entry * mailboxes_slab;

void
init_mailboxes () {
  
  mailboxes_slab = create_slab (sizeof (mailbox));
  
}

/*
 * Create a mailbox of size 'size' in PAGE_SIZE's,
 * able to receive from PID recv_pid (or all of -1)
 */
mailbox *
create_mailbox (context_t * c, int size, int recv_pid) {

  memory_region * mr = create_region (c->space, 0,
              0,
				      MR_TYPE_IPC, 
				      MR_ATTR_RO,
				      0);
  if (!mr) return 0;
  expand_region (mr, size / 4096 + 1);
  mailbox * mb = (mailbox *) allocate_from_slab(mailboxes_slab);
  if (!mb) {
    return 0;
  }

  mb->recv_pid = recv_pid;
  mb->size = size;
  mb->mb_mr = mr;
  mb->first = 1;
  mb->last = 0;
  mb->mutex = 0;
  
  mb->next = c->mailboxes;
  c->mailboxes = mb;
  
  return mb;

}

message * 
next_message (mailbox * mb) {
  
  if (test_and_set(1, &mb->mutex)) {
    bootvideo_printf ("Mailbox locked!\n");
    return (message *) MAILBOX_ERR_LOCKED;
  }

  message * m;
  if (mb->first == mb->last) {
    test_and_set (0, &mb->mutex);
    return (message *) MAILBOX_ERR_EMPTY;
  }
  if (mb->first == (mb->last + 1 % mb->size)) {
    test_and_set (0, &mb->mutex);
    return (message *)MAILBOX_ERR_EMPTY;
  }
  
  m = &((message *)(mb->mb_mr->virtual_address))[(mb->last + 1) % mb->size];
  mb->last = (mb->last + 1) % mb->size;

  test_and_set (0, &mb->mutex);

  return m;
}
  

int send_message (message * m, int kmode) {

  /*Find the destination mailbox*/
  
  context_t * c = get_process (m->dest_pid);
  if (!c) {
    return MAILBOX_ERR_PERM;
  }
  mailbox * mb = c->mailboxes;

  if ((m->src_pid != get_current_process ()) && !kmode) {
    return MAILBOX_ERR_PERM;
  }

  while (mb != 0) {
    if (mb->recv_pid == m->src_pid) break;
    mb = mb->next;
  }

  if (!mb) {
    mb = c->mailboxes;
    while (mb != 0) {
      if (mb->recv_pid == 0) break;
      mb = mb->next;
    }
  }

  if (!mb) return MAILBOX_ERR_PERM;
  if (test_and_set (1, &mb->mutex)) {
    return MAILBOX_ERR_LOCKED;
  }

  if ((mb->first + 1) % mb->size == mb->last) {
    test_and_set (0, &mb->mutex);
    return MAILBOX_ERR_FULL;
  }
  
  unsigned long addr = mb->mb_mr->virtual_address + mb->first * sizeof(message);
  
  int index = mb->first % (PAGE_SIZE / sizeof(message));

  message * list = (message *)map_user_virtual_to_kernel (c, addr);
  unsigned int i;
  for (i = 0; i < sizeof (message); ++i) {
    *((char*)&list[index] + i) = *((char*)m + i);
  }


  mb->first = (mb->first + 1) % mb->size;

  test_and_set (0, &mb->mutex);

  free_kernel_virtual_page ( (int)list);

  if (c->status == PROCESS_STATUS_WAITING) {
    c->status = PROCESS_STATUS_RUNNING;
  }

  return 0;
  

}
