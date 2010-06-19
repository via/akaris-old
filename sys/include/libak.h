#ifndef LIBAK_H
#define LIBAK_H

#include <i386/mailbox.h>

void puts (char *);
void itoa (char *, int, int);
void sprintf (char *, const char *, ...);
mailbox_t * ak_mailbox_create (int, int);
int ak_mailbox_send (message_t *);
message_t * ak_mailbox_receive (mailbox_t *);
void ak_block_on_message ();

void memcpy (void *, void *, unsigned int);

void ak_outb (unsigned short port, unsigned char);
unsigned char ak_inb (unsigned short port);
void ak_link_irq (int);

#endif
