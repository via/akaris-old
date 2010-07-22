#ifndef I386_KQUEUE_H
#define I386_KQUEUE_H

#include <i386/types.h>
#include <mutex.h>
#include <i386/interrupt.h>


typedef enum {
  KQUEUE_SUCCESS,
  KQUEUE_ERR_MEM,
  KQUEUE_ERR_LOCK,
  KQUEUE_ERR_EXIST,
  KQUEUE_ERR_PERM,
} kqueue_error;

typedef enum {                       
  KEVENT_FIFO,
  KEVENT_PROC,
} kevent_filter_t;

typedef enum {
  KEVENT_FLAG_FIFO_READABLE,
  KEVENT_FLAG_FIFO_WRITABLE,
  KEVENT_FLAG_PROC_QUIT,
} kevent_flag_t;

typedef struct kevent {
  kevent_filter_t filter;
  kevent_flag_t flag;
  enum {
    KEVENT_STATUS_ENABLED,
    KEVENT_STATUS_DISABLED,
  } status;
  struct kevent *next;
  struct kevent *hook_next;
  struct kqueue *parent;
  struct kqevent_hook *hook;
  uint32 ident;
  mutex_t triggered;
} kevent_t;

typedef struct kqevent_hook {
  int (*add_event) (struct kevent *ke);
  int (*delete_event) (struct kevent *ke);
} kqevent_hook_t;
         


typedef struct kqueue {
  uint32 pid;
  uint32 kqueue_id;
  kevent_t *event_list;

  struct kqueue *next;
} kqueue_t;

void initialize_kqueues();
kqueue_error create_kqueue (uint32 *, uint32 mypid);
kqueue_error kqueue_event (uint32 kqueue_id, uint32 mypid, uint32 ident, kevent_filter_t filter,
    kevent_flag_t flag);
kqueue_error kqueue_block (uint32 kqueue_id, isr_regs *regs);
kqueue_error kqueue_poll (uint32 kqueue_id, kevent_t *, uint32 *max_changes);
void kqueue_trigger_event (kevent_t *);
void kqueue_untrigger_event (kevent_t *);


#endif
