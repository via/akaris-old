#ifndef I386_CONTEXT_H
#define I386_CONTEXT_H

#include <config.h>
#include <i386/physical_memory.h>
#include <i386/slab.h>
#include <i386/paging.h>
#include <i386/bootvideo.h>

#define MR_TYPE_STACK 1
#define MR_TYPE_CORE 2
#define MR_TYPE_LIBRARY 3
#define MR_TYPE_ANON 4
#define MR_TYPE_IPC 5
#define MR_TYPE_SENTINAL 6
#define MR_TYPE_FREE 7
#define MR_TYPE_KERNEL 8


#define MR_ATTR_COW 1 /*parameter is pointer to other region*/
#define MR_ATTR_RO 2


struct memory_region_t {
  unsigned long virtual_address;
  int length;  /*In pages*/
  int type;
  int attributes;
  int parameter;
  struct address_space_t *parent;
  struct memory_region_t *next;
};

struct address_space_t {
  pde* cr3;
  pde* virt_cr3;

  struct memory_region_t *first;
  struct memory_region_t *last;

  struct memory_region_t *core;
  struct memory_region_t *stack;
  /*  struct memory_region_t *mappings;*/
};

typedef struct memory_region_t memory_region;
typedef struct address_space_t address_space;

void            init_address_space_system();

address_space * create_address_space();

memory_region * determine_memory_region(address_space *, unsigned long address);
int             expand_region(memory_region*, int size);

memory_region * create_region(address_space *, int flags, int attr, int param);
int             map_region(memory_region *, int phys, int length);

#endif
