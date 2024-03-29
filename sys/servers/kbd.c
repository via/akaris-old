#include <libak.h>
#include <servers/kbd.h>
#include <common/kqueue.h>

char buf[50];

void mod_start () {
  kfifo_error e;
  int scancode_in;
  char ready;
  uint32 irq_fifo;
  uint32 devnode_fifo;
  uint32 conn_fifos[2];
  struct kevent eventlist[2];
  uint32 nevents = 1;

  ak_link_irq (&irq_fifo, 33);
  ak_register ("os:dev:keyboard");
  ak_accept ("os:dev:keyboard", &devnode_fifo);
  scancode_in = ak_inb (0x60);
  puts ("Started KBD\n");

  uint32 kid;

  char str[50];

  ak_kqueue_create (&kid);
  while (1) {

    while ((e = ak_read (devnode_fifo, conn_fifos, 8)) != KFIFO_SUCCESS);
    ak_kqueue_event (kid, irq_fifo, KEVENT_FIFO, KEVENT_FLAG_FIFO_READABLE);
    while (1) {
      ak_kqueue_block (kid);
      ak_kqueue_poll (kid, &nevents, eventlist); 
      e = ak_read (irq_fifo, &ready, 1);
      scancode_in = ak_inb (0x60);
      ak_write (conn_fifos[1], &scancode_in, 1);
      sprintf (str, "Key received! :%d:\n", nevents);
      puts (str);
    }
  }
}


