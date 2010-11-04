#include <libak.h>

char buf[99];
uint32 kbd_fifos[2];

void mod_start () {

  volatile char * a = ak_mmap (0xB8000, 4096);
  uint32 pipes[2];
  
  /*kqueue stuff*/
  uint32 kid;
  uint32 nevents = 3;
  struct kevent elist[10];
  
  kfifo_error e;

  *(a + 8 * 160) = '&';

  puts ("Connecting to keyboard!\n");

  int c;
  for (c = 0; c < 100000; ++c);

  ak_pipe (pipes);

  ak_connect ("os:dev:keyboard", kbd_fifos);
  e = ak_kqueue_create (&kid);
  e = ak_kqueue_event (kid, kbd_fifos[1], KEVENT_FIFO, KEVENT_FLAG_FIFO_READABLE);
  e = ak_kqueue_event (kid, pipes[1], KEVENT_FIFO, KEVENT_FLAG_FIFO_READABLE);
  puts ("Start client!\n");

  while (1) {
    ak_write (pipes[1], "    ", 5);
    e = ak_kqueue_block (kid);
    ak_kqueue_poll (kid, &nevents, elist);
    sprintf (buf, "Key Recvd: %d\n", nevents);
    puts (buf);
  }

  

  


}

