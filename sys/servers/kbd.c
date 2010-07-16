#include <libak.h>
#include <servers/kbd.h>

char buf[50];

void mod_start () {
  kfifo_error e;
  int scancode_in;
  char ready;
  uint32 irq_fifo;
  uint32 devnode_fifo;
  uint32 conn_fifos[2];
  ak_link_irq (&irq_fifo, 33);
  ak_register ("os:dev:keyboard");
  ak_accept ("os:dev:keyboard", &devnode_fifo);
  puts ("Started KBD\n");


  while (1) {

    while ((e = ak_read (devnode_fifo, conn_fifos, 8)) != KFIFO_SUCCESS);
    while (1) {
      while (( e = ak_read (irq_fifo, &ready, 1)) != KFIFO_SUCCESS);
      scancode_in = ak_inb (0x60);
      ak_write (conn_fifos[1], &scancode_in, 1);
      puts ("Key received!\n");
    }
  }
}


