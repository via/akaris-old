#ifndef I386_MAILBOX_H
#define I386_MAILBOX_H
/*Forward decl*/
struct memory_region;
struct context;

typedef struct {
  int src_pid;
  int dest_pid;
  short size;
  char payload[128];
} message_t;
 

typedef struct mailbox {
  struct mailbox *next;
  int recv_pid;
  int size; /*In pages*/
  struct memory_region * mb_mr;
  int first, last;
  int mutex;
} mailbox_t;


#define MAILBOX_ERR_EMPTY 0
#define MAILBOX_ERR_PERM 1
#define MAILBOX_ERR_FULL 2
#define MAILBOX_ERR_LOCKED 3


void      init_mailboxes ();
mailbox_t * create_mailbox (struct context *, int size, int recv_pid);
message_t * next_message (mailbox_t *);
int       send_message (message_t *, int);




#endif /*I386_MAILBOX_H*/
