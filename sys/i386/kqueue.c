/*! \file kqueue.c
 *
 * This is a half-assed implementation of kqueue from freebsd, except its not
 * really the same.  Different api, same purpose.  One day i'll make it exactly
 * like freebsd's kqueue.
 *
 * One creates a kqueue from userspace with create_kqueue().  You can then add
 * individual trigger events using kqueue_event().  For example, you could
 * create a kevent with a identifier being a FIFO, and the filter being
 * KEVENT_FIFO, and the flag being KEVENT_FLAG_FIFO_READABLE.  Then if you call
 * kqueue_block, it will block the process until the specified fifo is readable.
 */

#include <mutex.h>
#include <config.h>
#include <i386/types.h>
#include <i386/slab.h>
#include <i386/process.h>
#include <i386/context.h>
#include <i386/kfifo.h>
#include <i386/kqueue.h>

static slab_entry_t * kqueue_slab;
static slab_entry_t * kevent_slab;

static mutex_t kqueue_lock; /*TODO: Big locks suck cock*/
static kqueue_t * kqueue_list;
static uint32 next_kqueue_id = 1;

static kqevent_hook_t fifo_hook = {
  kfifo_add_kqueue_event,
  kfifo_delete_kqueue_event,
};

/*! \brief Initializes Kqueue API
 */
void
initialize_kqueues () {

  kqueue_slab = create_slab (sizeof (kqueue_t));
  kevent_slab = create_slab (sizeof (kevent_t));

  next_kqueue_id = 0;

}

/*! \brief Creates a kqueue.
 *
 * Creates a kqueue object and attaches it to the kqueue list.
 * \param id Place to store the kqueue id
 * \param mypid Current Process's PID
 * \returns Kqueue error code
 */
kqueue_error
create_kqueue (uint32 *id, uint32 mypid) {

  kqueue_t * newkq;

  if (test_and_set (1, &kqueue_lock) != 0) {
    return KQUEUE_ERR_LOCK;
  }

  newkq = (kqueue_t *) allocate_from_slab (kqueue_slab);
  if (NULL == newkq) {
    test_and_set (0, &kqueue_lock);
    return KQUEUE_ERR_MEM;
  }

  newkq->pid = mypid;
  newkq->kqueue_id = next_kqueue_id++;
  *id = newkq->kqueue_id; 

  newkq->next = kqueue_list;
  kqueue_list = newkq;
  newkq->event_list = NULL;

  test_and_set (0, &kqueue_lock);

  return KQUEUE_SUCCESS;
}

/* \brief Adds a filter to an existing kqueue.
 *
 * Attempts to add a filter/flag combination to a kqueue.
 * When an event is added, it will will add itself to a hook's
 * list, and triggered will be set when an event occured.  This
 * way, when kqueue_block is called, it will know immediately
 * an event's status by checking triggered.
 *
 * \param kqueue_id The kqueue to add the event to.
 * \param mypid Calling process's pid.
 * \param ident Identifier for the event (pid, fifo, etc)
 * \param filter Specifies the meaning of ident.
 * \param flag Specifies what action is being looked for.
 * \returns Kqueue error code.
 */
kqueue_error
kqueue_event (uint32 kqueue_id, uint32 mypid, uint32 ident,
    kevent_filter_t filter, kevent_flag_t flag) {

  kevent_t * newev;
  kqueue_t * kq;

  if (test_and_set (0, &kqueue_lock) != 0) {
    return KQUEUE_ERR_LOCK;
  }

  newev = (kevent_t *) allocate_from_slab (kevent_slab);
  
  if (newev == NULL) {
    test_and_set (0, &kqueue_lock);
    return KQUEUE_ERR_MEM;
  }

  /*Find the kqueue first*/
  kq = kqueue_list;
  while (kq != NULL) {
    if (kq->kqueue_id == kqueue_id) break;
    kq = kq->next;
  }
  if (kq == NULL) {
    test_and_set (0, &kqueue_lock);
    deallocate_from_slab (kevent_slab, newev);
    return KQUEUE_ERR_EXIST;
  }

  if (kq->pid != mypid) {
    test_and_set (0, &kqueue_lock);
    deallocate_from_slab (kevent_slab, newev);
    return KQUEUE_ERR_PERM;
  }

  newev->filter = filter;
  newev->flag = flag;
  newev->status = KEVENT_STATUS_ENABLED;
  newev->parent = kq;
  newev->ident = ident;
  newev->triggered = 0;
  newev->next = kq->event_list;
  kq->event_list = newev;

  switch (newev->filter) {
    case KEVENT_FIFO:
      newev->hook = &fifo_hook;
      break;
    case KEVENT_PROC:
      /*Not implemented yet*/
      break;
  }

  newev->hook->add_event (newev);

  test_and_set (0, &kqueue_lock);
  return KQUEUE_SUCCESS;
}

void
kqueue_trigger_event (kevent_t * ke) {

  context_t *curproc;
  curproc = get_process (ke->parent->pid);

  ke->triggered = 1;
  


}

