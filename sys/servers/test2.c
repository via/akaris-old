
#include <libak.h>
#include <servers/kbd.h>

mailbox *mb;
message m;
message *in;
ak_kbd_request_t r;
ak_kbd_response_t *re;
void mcpy (char * d, char * s, int l);

void mod_start() {


  mb = ak_mailbox_create (30, 1);

  m.src_pid = 2;
  m.dest_pid = 1;
  r.type = AK_KBD_REQ_TYPE_ASCII;
  r.stop_delimiter = 1;
  mcpy (m.payload, (char*)&r, sizeof (r));
  /* ak_mailbox_send (&m);*/
  

  while (1) {

    ak_block_on_message ();
    in = ak_mailbox_receive (mb);
    
    

    sprintf (m.payload, "Received keypress %d\n", ((ak_kbd_response_t*)&(in->payload))->c);

  }
}
void mcpy (char * d, char * s, int l) {
  for (; l > 0; *d++ = *s++, --l);
}
