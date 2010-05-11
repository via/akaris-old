#ifndef I386_PIC_H
#define I386_PIC_H

int picAvailable();
void picRemapIRQs();

void outportb(unsigned short port, unsigned char data);
unsigned char inportb(unsigned short port);
#endif
