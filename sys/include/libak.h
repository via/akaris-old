#ifndef LIBAK_H
#define LIBAK_H

#include <i386/kfifo.h>

void puts (char *);
void itoa (char *, int, int);
void sprintf (char *, const char *, ...);

void memcpy (void *, void *, unsigned int);

void ak_outb (unsigned short port, unsigned char);
unsigned char ak_inb (unsigned short port);

void ak_pipe (uint32 *pipes);
void ak_read (uint32 pipe, void * buf, uint32 len);
void ak_write (uint32 pipe, void * buf, uint32 len);
#endif
