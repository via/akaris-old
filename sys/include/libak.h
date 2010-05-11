#ifndef LIBAK_H
#define LIBAK_H

#include <i386/mailbox.h>

void puts (char *);
void itoa (char *, int, int);
void sprintf (char *, const char *, ...);
mailbox * ak_mailbox_create (int, int);
int ak_mailbox_send (int, message *);
message * ak_mailbox_receive (mailbox *);
void ak_block_on_message ();

#endif
