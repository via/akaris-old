#include <i386/slab.h>



slab_entry slabs[MAX_SLABS];
/*Note, later for numa...slab table per cpu, different memory*/

void init_slabs() {
  int c;
  for (c = 0; c < MAX_SLABS; c++) {
    slabs[c].lock = 0;
    slabs[c].num_objects = 0;
    slabs[c].first_sbuf = (sbuf *) 0;
    slabs[c].first_free_sbuf = (sbuf *) 0;
  }
}

slab_entry * create_slab (int size) {
  int c;
  int i;

  if (size < 4) {
    return (slab_entry *) 0;  /*Null*/
  }

  for (c = 0; c < MAX_SLABS; c++) {
    if (slabs[c].first_sbuf == 0)
      /*We will use this slab*/
      break;
  }

  if (c == MAX_SLABS) return (slab_entry *) 0;

  slabs[c].size = size;
  slabs[c].first_sbuf = (sbuf *)(get_usable_kernel_virtual_page());
  slabs[c].first_sbuf->free_objects = (PAGE_SIZE - sizeof(sbuf)) / size - 1;
  slabs[c].num_objects = 0;
  slabs[c].first_free_sbuf = slabs[c].first_sbuf;
  
  slabs[c].first_sbuf->first_free = 0;
  slabs[c].first_sbuf->next = (sbuf *) 0;

  for (i = 0; i < slabs[c].first_sbuf->free_objects; i++) {
    int * t_object;
    t_object = (int*)(((void *)slabs[c].first_sbuf) + sizeof(sbuf) + (i * size));
    if (i == slabs[c].first_sbuf->free_objects - 1) {
      *t_object = -1;
    } else {
      *t_object = i + 1;
    }
  }

  return &slabs[c];
}
      
void delete_slab(slab_entry * se) {
  if (se->num_objects != 0) return;
  se->first_sbuf = (sbuf *) 0;
}

void * allocate_from_slab(slab_entry * se) {
  
  void * result;
  int new_next, i;

  while (test_and_set(1, &se->lock) != 1);

  if (se->first_free_sbuf->free_objects == 0) {
    /*Create and initialize another sbuf*/
    sbuf * new_sbuf = (sbuf *)(get_usable_kernel_virtual_page());
    new_sbuf->free_objects = (PAGE_SIZE - sizeof(sbuf)) / se->size - 1;
    new_sbuf->first_free = 0;
    new_sbuf->next = (sbuf *) 0;

    for (i = 0; i < new_sbuf->free_objects; i++) {
      int * t_object;
      t_object = (int*)(((void *)new_sbuf) + sizeof(sbuf) + (i * se->size));
      if (i == new_sbuf->free_objects - 1) {
	*t_object = -1;
      } else {
	*t_object = i + 1;
      }
    }

    se->first_free_sbuf->next = new_sbuf;
    se->first_free_sbuf = new_sbuf;

    
  }

  result = ((void*)(se->first_free_sbuf)) + sizeof(sbuf) +
    (se->first_free_sbuf->first_free * se->size);
  new_next = *((int *)result);
  se->first_free_sbuf->first_free = new_next;
  se->first_free_sbuf->free_objects--;
  se->num_objects++;

  while (test_and_set (0, &se->lock) != 0);
  return result;
}


    


  

      
   
