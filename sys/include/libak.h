#ifndef LIBAK_H
#define LIBAK_H

#include <i386/kfifo.h>
#include <i386/device_interface.h>
void puts (char *);
void itoa (char *, int, int);
void sprintf (char *, const char *, ...);

void memcpy (void *, void *, unsigned int);

void ak_outb (unsigned short port, unsigned char);
unsigned char ak_inb (unsigned short port);

void ak_pipe (uint32 *pipes);
kfifo_error ak_read (uint32 pipe, void * buf, uint32 len);
kfifo_error ak_write (uint32 pipe, void * buf, uint32 len);
kfifo_error ak_close (uint32 pipe);
int ak_fork ();
dev_error ak_register (char *);
dev_error ak_connect (char *, uint32 * fifos);
dev_error  ak_accept (char *, uint32 * fifo);
#endif
