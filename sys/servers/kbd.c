#include <libak.h>
#include <servers/kbd.h>

char buf[50];

struct kmap {
  char ascii[2];
} asciimap[] = {
  {{'\0' ,'\0'}}, /*0*/
  {{'\0' ,'\0'}},
  {{'1','!'}},
  {{'2','@'}},
  {{'3','#'}},
  {{'4','$'}},
  {{'5','%'}},
  {{'6','^'}},
  {{'7','&'}},
  {{'8','*'}},
  {{'9','('}}, /*10*/
  {{'0',')'}},
  {{'\0','\0'}},
  {{'\0','\0'}},
  {{'\0','\0'}},
  {{'\0','\0'}},
  {{'q','Q'}},
  {{'w','W'}},
  {{'e','E'}},
  {{'r','R'}},
  {{'t','T'}}, /*20*/
  {{'y','Y'}},
  {{'u','U'}},
  {{'i','I'}},
  {{'o','O'}},
  {{'p','P'}},
  {{'\0','\0'}},
  {{'\0','\0'}},
  {{'\n','\0'}},
  {{'\0','\0'}},
  {{'a','A'}}, /*30*/
  {{'s','S'}},
  {{'d','D'}},
  {{'f','F'}},
  {{'g','G'}},
  {{'h','H'}},
  {{'j','J'}},
  {{'k','K'}},
  {{'l','L'}},
  {{',',';'}},
  {{'\0','\0'}}, /*40*/
  {{'\0','\0'}},
  {{'\0','\0'}},
  {{'\0','\0'}},
  {{'z','Z'}},
  {{'x','X'}},
  {{'c','C'}},
  {{'v','V'}},
  {{'b','B'}},
  {{'n','N'}},
  {{'m','M'}}, /*50*/
  {{'\0','\0'}},
  {{'\0','\0'}},
  {{'\0','\0'}},
  {{'\0','\0'}},
  {{'\0','\0'}},
  {{'\0','\0'}},
  {{' ',' '}},
  {{'\0','\0'}},
  {{'\0','\0'}},
  {{'\0','\0'}},
  {{'\0','\0'}},
  {{'\0','\0'}},

};

void mcpy (char * d, char * s, int l);

void mod_start () {
  
  mailbox_t * kern = ak_mailbox_create (10, 0);
  message_t * m;
  message_t out;
  ak_kbd_request_t akr;
  ak_kbd_response_t akrp;
  int dest_pid = 0;
  int c;
  char a;
  ak_link_irq (33);
  puts ("Started KBD\n");

  short shift = 0;

  while (1) {
    ak_block_on_message ();
    m = ak_mailbox_receive (kern);
    if (m->src_pid == -1) {
      c = ak_inb (0x60);
      a = '\0';
      if ((c == 54) || (c == 42)) {
	shift = 1;
      } else if ((c == 54 + 128) || ( c == 42 + 128)) {
	shift = 0;
      } else {
	if (c < 60) {
	  a = asciimap[(int)c].ascii[shift];
	} else {
	  a = '\0';
	}
      }
      if (dest_pid == 0) {
	puts ("Nobody hooked!  ");
	if (a != '\0') {
	  sprintf (buf, "Scancode Received: %c\n", a);
	  puts (buf);
	}
      } else {
	if (a != '\0') {
	  akrp.c = a;
	  if (a == akr.stop_delimiter) {
	    akrp.complete = 1;
	    out.dest_pid = dest_pid;
	    dest_pid = 0;
	  } else {
	    akrp.complete = 0;
	    out.dest_pid = dest_pid;
	  }
	  out.src_pid = 1;
	  mcpy (out.payload, (char*)&akrp, sizeof(akrp));
	  
	  ak_mailbox_send (&out);
	}
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
