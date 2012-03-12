#ifndef LIBAK_H
#define LIBAK_H

#include <common/kfifo.h>
#include <common/device_interface.h>
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
dev_error ak_link_irq (uint32 *fifo, uint8 irq);
void * ak_mmap (unsigned long phys, unsigned long size);
kqueue_error ak_kqueue_create (uint32 *id);
kqueue_error ak_kqueue_event (uint32 kid, uint32 id, kevent_filter_t filter, kevent_flag_t flag);
kqueue_error ak_kqueue_block (uint32 id);
kqueue_error ak_kqueue_poll (uint32 id, uint32 *nevents, struct kevent *eventlist);
#endif
