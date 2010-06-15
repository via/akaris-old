
#include <libak.h>
char buf[100];

void mod_start() {
  message_t *m;
  mailbox_t * mb = ak_mailbox_create (10, 0);
  

  
  while (1) {
    int i;

    for (i = 0; i < 5000; i++);
    
    while (1)
      {
	ak_block_on_message ();
	m = ak_mailbox_receive (mb);
      if (m != 0) break;
    }
    sprintf (buf, "message received: %s\n",  m->payload);
    puts (buf);

  }


  
}

