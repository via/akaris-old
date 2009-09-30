#ifndef I386_BOOTVIDEO
#define I386_BOOTVIDEO

#define NUM_COLS 80
#define NUM_ROWS 25






/*Function prototypes */
    
void bootvideo_cls();
void bootvideo_putc(char);
void itoa (char*, int, int);
void bootvideo_printf (const char *format, ...);
void memset(int * dest, int value, int length);
void memcpy(char *, char *, int);
#endif
