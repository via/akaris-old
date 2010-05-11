
#include <libak.h>

char gstr[] = "pid2 iterate\n";
char pay[] = "From 2: Payload!\n";
void mod_start() {
  int c = 0;
  while (1) {
    int i;
    for (i = 0; i < 10000000; i++);

    puts (gstr);

      

    if (c % 3 == 0) {
      ak_mailbox_send (1, (message *)pay);
    }
    ++c;    
  }
}
