#ifndef I386_PHYSICAL_MEMORY_H
#define I386_PHYSICAL_MEMORY_H

#include <i386/multiboot.h>
#include <mutex.h>

typedef struct {
  int start_page;
  int domain_id;
  int * bitmap;
  mutex_t lock;
  int total_pages;
  int free_pages;
} memory_domain;

void  initialize_memory(multiboot_info_t *);
int allocate_page(int domain); /*returns page number*/
void set_page_status(int domain, int pagenumber, int status); /*1 = allocated*/
#endif
