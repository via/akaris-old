#ifndef I386_BOOTVIDEO
#define I386_BOOTVIDEO

#define NUM_COLS 80
#define NUM_ROWS 25


#include <common/types.h>



/*Function prototypes */
    
void bootvideo_cls();
void bootvideo_putc(char);
void itoa (char*, int, int);
void bootvideo_printf (const char *format, ...);
void memset(int * dest, int value, int length);
void memcpy(char *, const char *, int);
int strncmp (const char *, const char *, uint32);
int strlen (const char *, uint32);
#endif
