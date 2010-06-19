
#include <libak.h>
#include <servers/kbd.h>

mailbox_t *mb;
message_t m;
message_t *in;
ak_kbd_request_t r;
ak_kbd_response_t *re;


void mod_start() {
  char obuf[100];
  int i;
  for (i = 0; i < 1000000; ++i);
  mb = ak_mailbox_create (30, 1);

  m.src_pid = 2;
  m.dest_pid = 1;
  r.type = KB_REQUEST_EXCLUSIVE;
  memcpy (m.payload, (char*)&r, sizeof (r));
  ak_mailbox_send (&m);

  while (1) {
    while (1) {
      ak_block_on_message ();
      in = ak_mailbox_receive (mb);
      re = (ak_kbd_response_t *) in->payload;
      sprintf (obuf, "Received: %d %d\n", re->escape, re->scancode);
      puts (obuf);
    }
     
  }
}
