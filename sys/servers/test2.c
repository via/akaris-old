
#include <libak.h>
#include <servers/kbd.h>

mailbox_t *mb;
message_t m;
message_t *in;
ak_kbd_request_t r;
ak_kbd_response_t *re;
void mcpy (char * d, char * s, int l);



void mod_start() {
  char buf[100];
  char obuf[100];
  int i;
  for (i = 0; i < 1000000; ++i);
  mb = ak_mailbox_create (30, 1);

  m.src_pid = 2;
  m.dest_pid = 1;
  r.type = AK_KBD_REQ_TYPE_ASCII;
  r.stop_delimiter = '\n';
  mcpy (m.payload, (char*)&r, sizeof (r));
  ak_mailbox_send (&m);

  while (1) {
    int c = 0;
    while (1) {
      ak_block_on_message ();
      in = ak_mailbox_receive (mb);
      if (((ak_kbd_response_t*)&(in->payload))->complete) break;
      buf[c] = ((ak_kbd_response_t*)&(in->payload))->c;
      ++c;
    }
    buf[c] = '\0';
    
    

    sprintf (obuf, "Received keypress %s\n", buf);
    puts (obuf);
    
  }
}
void mcpy (char * d, char * s, int l) {
  for (; l > 0; *d++ = *s++, --l);
}
