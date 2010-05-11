
#include <libak.h>

char gstr[] = "pid2 iterate\n";
message m;

void mod_start() {
  int c = 0;
  while (1) {
    int i;
    for (i = 0; i < 10000000; i++);

    puts (gstr);

    m.src_pid = 2;
    m.dest_pid = 1;
    
    if (c % 3 == 0) {
      sprintf (m.payload, "payload # %d\n", c);
      /* ak_mailbox_send (&m);*/
    }
    ++c;    
  }
}
