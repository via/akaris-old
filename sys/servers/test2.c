char mystr[] = "Hello from userspace!\n";

void test1() {
  while (1) {
    int i;
    for (i = 0; i < 200000; i++);
    __asm__("movl $1, %%eax\n"
	    "movl %0, %%edx\n"
	    "int $0x80" : : "d" (mystr));
    test1();
  }
}
