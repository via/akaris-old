#ifndef I386_MAILBOX_H
#define I386_MAILBOX_H

#include <i386/process.h>
#include <i386/context.h>
#include <mutex.h>

struct mailbox_t {
  struct mailbox_t *next;
  int recv_pid;
  int size; /*In pages*/
  memory_region * mb_mr;
  int first, last;
  int mutex;
};

struct message_t {
  char payload[128];
};

typedef struct message_t message;
typedef struct mailbox_t mailbox;

void      init_mailboxes ();
mailbox * create_mailbox (context_t *, int size, int recv_pid);
message * next_message (mailbox *);
int       send_message (int pid, message *);




#endif /*I386_MAILBOX_H*/
