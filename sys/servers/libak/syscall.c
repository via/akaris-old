/* Userspace syscall.h
 * Implements easy to use functions to do stuff with the kernel */

#include <i386/mailbox.h>


void puts(char * s) {
  __asm__("movl $1, %%eax\n"
	  "int $0x80" : : "d" (s));
}

void ak_outb (unsigned short port, unsigned char val) {
  int reg = (val << 16) + port;
  __asm__("movl $9, %%eax\n"
	  "int $0x80" : : "d" (reg));
}
unsigned char ak_inb (unsigned short port) {
  int reg = 0xFF000000 | (port);

  __asm__("movl $9, %%eax\n"
	  "int $0x80" : "=d" (reg): "d" (reg));
  return (unsigned char)(0x000000FF & reg);
}
  
void ak_link_irq (int irq) {
  __asm__("movl $8, %%eax\n"
	  "int $0x80" : : "d" (irq));
}

mailbox * ak_mailbox_create (int max, int pid_filter) {
  mailbox * m;

  __asm__("movl $3, %%eax\n"

	"int $0x80" : "=d" (m) : "c" (pid_filter), "d" (max));
  
  return m;
}

void ak_block_on_message () {
  __asm__("movl $7, %%eax\n"
	  "int $0x80" : :);
}

int ak_mailbox_send (message * m) {

  int ret;
  __asm__("movl $4, %%eax\n"
	"int $0x80" : "=d" (ret) : "d" (m));
  return ret;
}

message * ak_mailbox_receive (mailbox * m) {
  message * msg;
  __asm__("movl $5, %%eax\n"
	  "int $0x80" : "=d" (msg) : "d" (m));
  return msg;
}

void itoa (char *buf, int base, int d)
{
  char *p = buf;
  char *p1, *p2;
  unsigned long ud = d;
  int divisor = 10;
  
  /* If %d is specified and D is minus, put `-' in the head. */
  if (base == 'd' && d < 0)
    {
      *p++ = '-';
      buf++;
      ud = -d;
    }
  else if (base == 'x')
    divisor = 16;
  
  /* Divide UD by DIVISOR until UD == 0. */
  do
    {
      int remainder = ud % divisor;
      
      *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
         }
  while (ud /= divisor);
  
  /* Terminate BUF. */
  *p = 0;
  
  /* Reverse BUF. */
  p1 = buf;
  p2 = p - 1;
  while (p1 < p2)
    {
      char tmp = *p1;
      *p1 = *p2;
      *p2 = tmp;
      p1++;
      p2--;
    }
}


void sprintf (char * s, const char *format, ...)
{
  char *out = s;
  char **arg = (char **) &format;
  int c;
  char buf[20];
  
  arg++;
  
  while ((c = *format++) != 0)
    {
      if (c != '%')
	*(out++) = (c);
      else
	{
	  char *p;
	  
	  c = *format++;
	  switch (c)
	    {
	    case 'd':
	    case 'u':
	    case 'x':
	      itoa (buf, c, *((int *) arg++));
	      p = buf;
	      goto string;
	      break;
	      
	    case 's':
	      p = *arg++;
	      if (! p)
		p = "(null)";
	      
	    string:
	      while (*p)
		*(out++) =  (*p++);
	      break;
	      
	    default:
	      *(out++) = (*((int *) arg++));
	      break;
	    }
	}
    }

  *(out++) = '\0';
}
