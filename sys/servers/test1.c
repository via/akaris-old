
void mod_start() {
  char *m;
  int mb;    
  __asm__("movl $3, %%eax\n"
	  "movl %0, %%edx\n"
	  "int $0x80" : "=d"(mb) : "d" (5));
  
  while (1) {
    int i;

    for (i = 0; i < 5000; i++);
    
    while (1);
      {
      __asm__("movl $5, %%eax\n"
	      "movl %0, %%edx\n"
	      "int $0x80" :"=d"(m) : "d" (mb));
      if (m != 0) break;
    }
    __asm__("movl $1, %%eax\n"
	    "movl %0, %%edx\n"
	    "int $0x80" : : "d" (m));
  }

  
}

