#include <libak.h>
#include <servers/kbd.h>

char buf[50];

void mcpy (char * d, char * s, int l);

void mod_start () {
  
  mailbox * kern = ak_mailbox_create (10, 0);
  message * m;
  message out;
  ak_kbd_request_t akr;
  ak_kbd_response_t akrp;
  int dest_pid = 0;
  char c;
  ak_link_irq (33);
  puts ("Started KBD\n");

  while (1) {
    ak_block_on_message ();
    m = ak_mailbox_receive (kern);
    if (m->src_pid == -1) {
      c = ak_inb (0x60);
      if (dest_pid == 0) {
	puts ("Nobody hooked!  ");
	sprintf (buf, "Scancode Received: %d\n", c);
	puts (buf);
      } else {
	akrp.c = c;
	if (c == akr.stop_delimiter) {
	  akrp.complete = 1;
	  dest_pid = 0;
	} else {
	  akrp.complete = 0;
	}
	out.src_pid = 1;
	out.dest_pid = dest_pid;
	mcpy (out.payload, (char*)&akrp, sizeof(akrp));
	ak_mailbox_send (&out);
      }
    } else {
      mcpy ((char*)&akr,m->payload, sizeof(akr));
      if (akr.type == AK_KBD_REQ_TYPE_ASCII) {
	dest_pid = m->src_pid;
	puts ("Loadings");
      }
    }
  }
}

void mcpy (char * d, char * s, int l) {

  for (; l > 0; *d++ = *s++, --l);

}
