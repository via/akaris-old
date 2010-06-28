/* Userspace syscall.h
 * Implements easy to use functions to do stuff with the kernel */

#include <i386/syscall.h>

void puts(char * s) {
  __asm__("int $0x80" : :"a" (SYSCALL_KDEBUG), "d" (s));
}

void ak_pipe (uint32 *pipes) {
  __asm__("int $0x80" :"=d" (pipes[0]), "=c" (pipes[1]) : "a" (FIFO_PIPE));
}

int ak_fork () {
  int newpid;
   __asm__("int $0x80" :"=d" (newpid) : "a" (FORK));
   return newpid;
}  

kfifo_error ak_write (uint32 pipe, void * buf, uint32 length) {
  fifo_op_t fop;
  fop.buf = buf;
  fop.fifo_id = pipe;
  fop.length = length;
  fop.operation = FIFO_OP_WRITE;
  __asm__("int $0x80" : : "a" (FIFO_OP), "d" (&fop));
  return fop.err;
}
kfifo_error ak_read (uint32 pipe, void * buf, uint32 length) {
  fifo_op_t fop;
  fop.buf = buf;
  fop.fifo_id = pipe;
  fop.length = length;
  fop.operation = FIFO_OP_READ;
  __asm__("int $0x80" : : "a" (FIFO_OP), "d" (&fop));
  return fop.err;
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

void memcpy (void * dst, void * src, unsigned int length) {
  
  char * s = (char *) src;
  char * d = (char *) dst;

  while (length) {
    *d++ = *s++;
    --length;
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
