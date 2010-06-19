#include <libak.h>
#include <servers/kbd.h>

char buf[50];

void mod_start () {
  
  mailbox_t * kern = ak_mailbox_create (10, 0);
  message_t * request_in;
  message_t out;
  ak_kbd_request_t akr;
  ak_kbd_response_t akrp;
  int dest_pid = 0;
  int scancode_in;
  int escape;

  ak_link_irq (33);
  puts ("Started KBD\n");


  while (1) {
    ak_block_on_message ();
    request_in = ak_mailbox_receive (kern);
    if (request_in->src_pid == -1) {
      scancode_in = ak_inb (0x60);
      if (KB_ESCAPE == scancode_in) {
        escape = scancode_in;
      } else {
        if (dest_pid != 0) { 
          out.src_pid = 1;
          out.dest_pid = dest_pid;
          akrp.scancode = scancode_in;
          akrp.escape = escape;
          memcpy (out.payload, (char*)&akrp, sizeof(akrp));
          ak_mailbox_send (&out);
          escape = 0;
        }
      }
    } else {
      memcpy ((char*)&akr,request_in->payload, sizeof(akr));
      if (akr.type == KB_REQUEST_EXCLUSIVE) {
        dest_pid = request_in->src_pid;
        puts ("Keyboard Attached!\n");
      }
    }
  }
}

