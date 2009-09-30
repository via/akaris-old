#include <config.h>
#include <i386/pic.h>

#ifdef ALLOW_PIC

void outportb(unsigned short port, unsigned char data)
{
    __asm__ __volatile__ ("outb %1, %0" : : "dN" (port), "a" (data));
}


int picAvailable() {
  return 1;
}

void picRemapIRQs() {

  outportb(0x20, 0x11);
  outportb(0xA0, 0x11);
  outportb(0x21, 0x20);
  outportb(0xA1, 0x28);
  outportb(0x21, 0x04);
  outportb(0xA1, 0x02);
  outportb(0x21, 0x01);
  outportb(0xA1, 0x01);
  outportb(0x21, 0x0);
  outportb(0xA1, 0x0);
}

#else

int picAvailable() {
  return 0;
}

void picRemapIRQs() {
  return;
}

#endif

  
