#include <config.h>
#include <mutex.h>
#include <i386/types.h>
#include <i386/bootvideo.h>
#include <i386/paging.h>
#include <i386/slab.h>



slab_entry_t slabs[MAX_SLABS];
/*Note, later for numa...slab table per cpu, different memory*/

void init_slabs() {
  int c;
  for (c = 0; c < MAX_SLABS; c++) {
    slabs[c].lock = 0;
    slabs[c].num_objects = 0;
    slabs[c].empty = (sbuf_t *) 0;
    slabs[c].partial = (sbuf_t *) 0;
    slabs[c].full = (sbuf_t *) 0;
  }
}

slab_entry_t * create_slab (int size) {
  int c;
  uint32 i;

  if (size < 4) {
    return (slab_entry_t *) 0;  /*Null*/
  }

  for (c = 0; c < MAX_SLABS; c++) {
    if (slabs[c].size == 0)
      /*We will use this slab*/
      break;
  }

  if (c == MAX_SLABS) return (slab_entry_t *) 0;

  slabs[c].size = size;
  slabs[c].empty = (sbuf_t *)(get_usable_kernel_virtual_page());
  slabs[c].num_objects = 0;
  
  slabs[c].empty->first_free = 0;
  slabs[c].empty->next = NULL;
  slabs[c].empty->num_free_objects = (PAGE_SIZE - sizeof (sbuf_t)) / size;
  slabs[c].full = NULL;
  slabs[c].partial = NULL;
  for (i = 0; i < slabs[c].empty->num_free_objects; i++) {
    int32 * t_object;
    t_object = (int32*)(((void *)slabs[c].empty) + sizeof(sbuf_t) + (i * size));
    if (i == slabs[c].empty->num_free_objects - 1) {
      *t_object = -1;
    } else {
      *t_object = i + 1;
    }
  }

  return &slabs[c];
}
      
void delete_slab(slab_entry_t * se) {
  if (se->num_objects != 0) return;
}

void * allocate_from_slab(slab_entry_t * se) {
  
  void * result;
  int new_next;
  uint32 i;


  sbuf_t *current;

  while (test_and_set(1, &se->lock) != 1);

  if (se->partial != NULL) {
    current = se->partial;
  } else if (se->empty != NULL) {
    current = se->empty;
  } else { /*Need to create another*/
    /*Create and initialize another sbuf*/
    sbuf_t * new_sbuf = (sbuf_t *)(get_usable_kernel_virtual_page());
    new_sbuf->num_free_objects = (PAGE_SIZE - sizeof(sbuf_t)) / se->size ;
    new_sbuf->first_free = 0;
    new_sbuf->next = NULL;

    for (i = 0; i < new_sbuf->num_free_objects; i++) {
      int32 * t_object;
      t_object = (int32*)(((void *)new_sbuf) + sizeof(sbuf_t) + (i * se->size));
      if (i == new_sbuf->num_free_objects - 1) {
        *t_object = -1;
      } else {
        *t_object = i + 1;
      }
    }

    se->empty = new_sbuf;
    current = new_sbuf;
    
  }


  result = ((void*)current) + sizeof(sbuf_t) +
    (current->first_free * se->size);
  new_next = *((int32 *)result);
  current->first_free = new_next;
  current->num_free_objects--;
  se->num_objects++;

  if (current->num_free_objects == 0) {
    sbuf_t * temp = se->partial;
    if (temp == current) {
      se->partial = temp->next;
    } else {
      while (temp->next != NULL) {
        if (temp->next == current) {
          temp->next = temp->next->next;
          break;
        }
      }
    }
    current->next = se->full;
    se->full = current;
  } else if (current->num_free_objects == (PAGE_SIZE - sizeof (sbuf_t)) / se->size - 1) {
    sbuf_t * temp = se->empty;
    if (temp == current) {
      se->empty = temp->next;
    } else {
      while (temp->next != NULL) {
        if (temp->next == current) {
          temp->next = temp->next->next;
          break;
        }
      }
    }
    current->next = se->partial;
    se->partial = current; 
  }

  while (test_and_set (0, &se->lock) != 0);
  return result;
}

  
    


  

      
   
