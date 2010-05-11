#include <libak.h>

char buf[50];

void mod_start () {
  
  mailbox * kern = ak_mailbox_create (10, 0);
  message * m;

  ak_link_irq (33);

  puts ("Started KBD\n");

  while (1) {
    ak_block_on_message ();
    m = ak_mailbox_receive (kern);
    puts ("Grabbing code...  ");
    sprintf (buf, "Scancode Received: %d\n", ak_inb (0x60));
    puts (buf);
  }
}
