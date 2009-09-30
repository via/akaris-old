

while (1) {
  int i;
  for (i = 0; i < 5000; i++);
  __asm__("int $1");
 }
