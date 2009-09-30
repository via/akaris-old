/*This somewhat complex file contains everything, ever,
 *for all physical memory allocation, including up, smp, and
 *numa systems.  Physical memory banks are broken into banks
 *and the cpu directory can point to various banks.  This
 *will allow for lock free access for different cpus to 
 *allocate memory*/


/*For time being, the infrastructure for a possible numa
 * numa system is being laid down, but its hardcoded
 * to only use one domain memory space for the time being*/

/*All allocations begin at 0x200000 for bitmaps*/

#include <i386/physical_memory.h>

memory_domain domains[MAX_MEMORY_DOMAINS];

void initialize_memory(multiboot_info_t * mb) {


  /*Some code goes here and determined what kind of
   *domains we have through ACPI, but now we just have one*/
  domains[0].domain_id = 0;
  domains[0].start_page = 0;
  domains[0].bitmap = (int*)0x200000;
  domains[0].lock   = 0;

  memory_map_t * mmap_entry;
  unsigned int page;

  for (mmap_entry = (memory_map_t*) mb->mmap_addr;
       (unsigned long)mmap_entry < mb->mmap_addr + mb->mmap_length;
       mmap_entry = (memory_map_t*)((unsigned long)mmap_entry +
				   mmap_entry->size + sizeof(mmap_entry->size))) {
    bootvideo_printf (" size = 0x%x, base_addr = 0x%x%x,"
	    " length = 0x%x%x, type = 0x%x\n",
	    (unsigned) mmap_entry->size,
	    (unsigned) mmap_entry->base_addr_high,
	    (unsigned) mmap_entry->base_addr_low,
	    (unsigned) mmap_entry->length_high,
	    (unsigned) mmap_entry->length_low,
	    (unsigned) mmap_entry->type);

    /*32 bits...so only using the low doublewords*/

    for (page = mmap_entry->base_addr_low >> 10;
	 page < ((mmap_entry->base_addr_low + mmap_entry->length_low) >> 10) + 1;
	 page++) {
      if (page <= (mb->mem_upper / 4)) {
	domains[0].total_pages++;
	if( mmap_entry->type == 1) {
	  set_page_status(0, page, 0);
	  domains[0].free_pages ++;
	} else {
	  set_page_status (0, page, 1);
	}
      }
    }
    
  }

 
  /*Now we need to make used all the areas of memory
   *that we're already using*/
  
  unsigned int first_free_page = (0x200000 / 4096) + domains[0].total_pages / (4096 * 32) + 2;
  for (page = 0; page < first_free_page; page++) {
    set_page_status(0, page, 1);
  }

  domains[0].free_pages -= first_free_page;

}

int allocate_page (int domain) {

  int found_int = 0;
  if (domains[domain].free_pages == 0) return 0;
  int * curint = domains[domain].bitmap;
  for (; curint <= (domains[domain].bitmap + 
		   domains[domain].total_pages / 32);
       curint++) {
    if ((unsigned)*curint != 0xFFFFFFFF) {
      found_int = 1;
      break;
    }
  }
  if (found_int == 0) {; /*We should never get here*/
    bootvideo_printf("Something fucked up. Freepages = %x\n", domains[domain].free_pages);
    while (1);
  }
  int offset;
  for (offset = 0; offset < 32; offset++) {
    if ( ((1 << offset) & *curint) == 0) break;
  }

  *curint |= (1 << offset);
  domains[domain].free_pages--;
  return (int)((curint - domains[domain].bitmap) * 32 + offset) + domains[domain].start_page;


    


}

void set_page_status (int d, int p, int s) {

  while (test_and_set(1, &domains[d].lock) != 1);

  int page = p - domains[d].start_page;

  int offset = page / 32; /*Offset into int array*/
  int bitoff = page % 32; /*offset into the int*/

  if (s == 1) {
    domains[d].bitmap[offset] |= (1 << bitoff);
  } else {
    domains[d].bitmap[offset] &= ~(1 << bitoff);
  }

  test_and_set(0, &domains[d].lock);

}
