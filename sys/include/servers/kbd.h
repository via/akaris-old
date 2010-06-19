#ifndef SERVERS_KBD_H
#define SERVERS_KBD_H

#define KB_ESCAPE 0x8e

typedef enum {
  KB_REQUEST_EXCLUSIVE,
} kb_request_type;

struct ak_kbd_request {
  kb_request_type type;
};

struct ak_kbd_response {
  unsigned char scancode;
  unsigned char escape;
};



typedef struct ak_kbd_request ak_kbd_request_t;
typedef struct ak_kbd_response ak_kbd_response_t;



#endif
