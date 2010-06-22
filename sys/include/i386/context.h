#ifndef I386_CONTEXT_H
#define I386_CONTEXT_H

/* Forward declarations for types*/
#include <i386/paging.h>

typedef enum {
  MR_TYPE_STACK,
  MR_TYPE_CORE,
  MR_TYPE_LIBRARY,
  MR_TYPE_ANON,
  MR_TYPE_IPC,
  MR_TYPE_SENTINEL,
  MR_TYPE_FREE,
  MR_TYPE_KERNEL,
} memory_region_type;


#define MR_ATTR_COW 1 /*parameter is pointer to other region*/
#define MR_ATTR_RO 2
#define MR_ATTR_PRIO_TOP 4

typedef struct memory_region {
  unsigned long virtual_address;
  int length;  /*In pages*/
  memory_region_type type;
  int attributes;
  unsigned long parameter;
  struct address_space *parent;
  struct memory_region *next;
  struct memory_region * sharedmem_next;
} memory_region_t;

typedef struct address_space {
  pde* cr3;
  pde* virt_cr3;

  struct memory_region *first;
  struct memory_region *last;

  struct memory_region *stack;
  /*  struct memory_region_t *mappings;*/
} address_space_t;

void            init_address_space_system();

address_space_t * create_address_space();

memory_region_t * determine_memory_region(address_space_t *, unsigned long address);
int             expand_region(memory_region_t *, int size);

memory_region_t * create_region(address_space_t *, unsigned long addr, int length, memory_region_type flags, int attr, unsigned long param);
int             map_region(memory_region_t *, int phys, int length);
void delete_region (memory_region_t *);
void context_print_mmap (memory_region_t *);

memory_region_t * clone_region (address_space_t *destspce, memory_region_t *region, int preserve);

#endif
