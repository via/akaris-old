#include <libak.h>
#include <servers/kbd.h>

char buf[50];

void mod_start () {
  kfifo_error e;
  int scancode_in;
  char ready;
  uint32 irq_fifo;
  ak_link_irq (&irq_fifo, 33);
  puts ("Started KBD\n");


  while (1) {
    while (( e = ak_read (irq_fifo, &ready, 1)) != KFIFO_SUCCESS);
    scancode_in = ak_inb (0x60);
    puts ("Key received!\n");
  }
}


