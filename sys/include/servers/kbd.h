#ifndef SERVERS_KBD_H
#define SERVERS_KBD_H


struct ak_kbd_request {
  int type;
  char stop_delimiter;
};

struct ak_kbd_response {
  char c;
  short complete;
};



typedef struct ak_kbd_request ak_kbd_request_t;
typedef struct ak_kbd_response ak_kbd_response_t;

#define AK_KBD_REQ_TYPE_ASCII 1


#endif
