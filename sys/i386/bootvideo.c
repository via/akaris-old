#include <i386/types.h>
#include <i386/bootvideo.h>

char * video = (char *)0xB8000;
int row = 0;
int col = 0;

void bootvideo_cls() {
  int *pos;
  for (pos = (int*)video; 
       pos < (int*)(video + (2 * NUM_COLS * NUM_ROWS) );
       pos++)
    *pos = 0x07000700;
  row = 0;
  col = 0;
}

void bootvideo_putc(char c) {
  if (c == '\n') {
    col = 0;
    row++;
  } else {
    video[(row * NUM_COLS + col) * 2] = c;
    col++;
  }
  if (col >= NUM_COLS) {
    col = 0;
    row++;
  }
  if (row >= NUM_ROWS) {
    int * pos;
    for (pos = (int*) video;
	 pos < (int*)(video + (2 * NUM_COLS * (NUM_ROWS - 1)));
	 pos++)
      *pos = *(pos + NUM_COLS / 2);
    row = NUM_ROWS - 1;
    for (pos = (int*)(video + (2 * NUM_COLS * (NUM_ROWS - 1)));
	 pos < (int*)(video + (2 * NUM_COLS * (NUM_ROWS)));
	 pos++)
	 *pos = 0x07000700;
	 
  }
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


void bootvideo_printf (const char *format, ...)
{
  char **arg = (char **) &format;
  int c;
  char buf[20];
  
  arg++;
  
  while ((c = *format++) != 0)
    {
      if (c != '%')
	bootvideo_putc (c);
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
      case 'l':
        itoa (buf, c, *((unsigned long *) arg++));
        p = buf;
        goto string;
        break;
	    case 's':
	      p = *arg++;
	      if (! p)
		p = "(null)";
	      
	    string:
	      while (*p)
		bootvideo_putc (*p++);
	      break;
	      
	    default:
	      bootvideo_putc (*((int *) arg++));
	      break;
	    }
	}
    }
}

void memset(int * dest, int value, int length) {

  int * cur;

  for (cur = dest; (int)cur < (int)dest + length; cur++)
    *cur = value;
}

void memcpy(char * dst, char *src, int length) {
  char *d, *s;

  for (d = dst, s = src;
       (int)s < (int)src + length;
       d++, s++) {
    *d = *s;
  }
}
