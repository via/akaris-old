#include <libak.h>

char buf[99];
char buf2[100];
char abuf[50];
uint32 pipe[2];
uint32 cpipe[2];
uint32 kbd_fifos[2];
void mod_start () {

  ak_pipe (pipe);
  ak_close (pipe[1]);

  volatile char * a = ak_mmap (0xB8000, 4096);
  *(a + 8 * 160) = '&';
  sprintf (abuf, "a = %x, *a = %d\n", a, *a);
  puts (abuf);
  
  int pid = ak_fork ();
  if (pid == 0) {
    puts ("Fork2\n");
    kfifo_error e;
    ak_register ("os:dev:test3");
    ak_accept ("os:dev:test3", &pipe[1]);
    while ((e = ak_read (pipe[1], cpipe, 8)) != 0);
    e = ak_write (cpipe[1], "Hello!\n", 7);

    e = ak_connect ("os:dev:keyboard", kbd_fifos);
    char sc;
    while (1) {
      while ( (e = ak_read (kbd_fifos[1], &sc, 1)) != KFIFO_SUCCESS);
      puts ("app received keypress\n");
    }
    while (1);


  } else {
    puts ("Fork1\n");

    kfifo_error e;
    uint32 newpipes[2];

    while ((e = ak_connect ("os:dev:test3", newpipes)) != 0);
    while ((e = ak_read (newpipes[1], buf2, 7)) != 0);

    puts (buf2);
    while (1);
  }


  while (1);

}


