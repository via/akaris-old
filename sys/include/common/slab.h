#ifndef I386_SLAB_H
#define I386_SLAB_H

#include <mutex.h>
#include <common/types.h>
/*The AK slab allocator is a very simple design:
 *A slab entry contains the address (page) of the first
 *buffer node.  It contains a link to the next page of
 *objects, if there is one (denoted by not being null).
 *The first_free is the index of the first free object.
 *Free objects always contain the relative address to the
 *next free object, essentially a large linked list of
 *free objects.  When full, another page of objects is 
 *allocated.
 */


typedef struct sbuf {
  uint32 num_free_objects;
  int32 first_free;
  struct sbuf *next;
} sbuf_t;

typedef struct {
  mutex_t lock;
  uint32 num_objects;
  sbuf_t *empty;
  sbuf_t *partial;
  sbuf_t *full;
  uint32 size;

} slab_entry_t;

void         init_slabs ();

slab_entry_t * create_slab(int size);
void         delete_slab(slab_entry_t *);

void *       allocate_from_slab(slab_entry_t *);
void         deallocate_from_slab(slab_entry_t *, void *);

void         dump_slab_info ();

#endif
