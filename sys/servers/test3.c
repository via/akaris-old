#include <libak.h>

char buf[99];
char buf2[100];
char abuf[50];
int i = 0;
uint32 pipe[2];
uint32 cpipe[2];
void blah () {
 while (1) {
    sprintf (buf, "Hello World #%d\n", i);
    ak_write (pipe[0], buf, 18);
    ak_read (pipe[0], buf, 18);
    puts (buf);
    ++i;
  }
}       
void mod_start () {

  ak_pipe (pipe);
  ak_close (pipe[1]);

  int pid = ak_fork ();
  if (pid == 0) {
    puts ("Fork2\n");
    kfifo_error e;
    ak_register ("os:dev:test3");
    ak_accept ("os:dev:test3", &pipe[1]);
    while ((e = ak_read (pipe[1], cpipe, 8)) != 0);
    e = ak_write (cpipe[1], "Hello!\n", 7);
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
  blah ();

}


