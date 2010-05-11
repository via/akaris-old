char gstr[] = "pid2 iterate\n";
char pay[] = "From 2: Payload!\n";
void mod_start() {
  int c = 0;
  while (1) {
    int i;
    for (i = 0; i < 10000000; i++);

    __asm__("movl $1, %%eax\n"
	    "movl %0, %%edx\n"
	    "int $0x80" : : "d" (gstr));
      

    if (c % 3 == 0) {
      
      
      __asm__("movl $4, %%eax\n"
	      "movl %0, %%edx\n"
	      "int $0x80" : : "d" (pay));
    }
    ++c;    
  }
}
