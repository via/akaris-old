/*context.c*/
#include <i386/context.h>
#include <i386/process.h>

slab_entry * address_slab;
slab_entry * regions_slab;


pte ** kpt;

void init_address_space_system() {

  address_slab = create_slab(sizeof(address_space));
  regions_slab = create_slab(sizeof(memory_region));

  
  kpt = get_kernel_page_tables();
}

address_space * create_address_space() {

  int c;
  address_space * as = (address_space *)allocate_from_slab(address_slab);
  memory_region * core;
  memory_region * stack;
  memory_region * free;


  pde * pages;


  as->first = (memory_region*)allocate_from_slab(regions_slab);
  as->last = (memory_region*)allocate_from_slab (regions_slab);
  
  as->first->type = MR_TYPE_SENTINEL;
  as->first->next = as->last;
  as->last->type  = MR_TYPE_SENTINEL;
  as->last->next = (memory_region *) 0;


  core = (memory_region *)allocate_from_slab(regions_slab);
  stack= (memory_region *)allocate_from_slab(regions_slab);
 
  pages = (pde*)get_usable_kernel_virtual_page();
  
  memset ( (int*)pages, 0, PAGE_SIZE);
  /*Link in the kernel page directories*/
  
  for (c = 0; c < NUM_KERNEL_PDES; c++) {
    set_pde(pages + c, (int)kpt[c], PTE_PRESENT_BIT | PTE_RW_BIT |
	    PTE_GLOBAL_BIT);
  }

  as->cr3 = (pde*)get_physical_address_from_kernel_virtual((int)pages);
  as->virt_cr3 = pages;
  as->core = core;
  as->stack = stack;

  core->virtual_address = 0x40000000;
  core->length = 0;
  core->type = MR_TYPE_CORE;
  core->attributes = 0;
  core->parent = as;
  core->next = as->last;

  stack->virtual_address = 0xC0000000;
  stack->length = 0;
  stack->type = MR_TYPE_STACK;
  stack->attributes = 0;
  stack->parent = as;
  stack->next = core;
  
  free = (memory_region *)allocate_from_slab(regions_slab);
  free->virtual_address = (4096 * 1024) * NUM_KERNEL_PDES;
  free->length = (0x40000000 - ((4096 * 1024) * NUM_KERNEL_PDES)) / 4096;
  free->attributes = 0;
  free->type = MR_TYPE_FREE;
  free->parent = as;
  free->next = stack;
  as->first->next = free;
  
  free = (memory_region *)allocate_from_slab(regions_slab);
  free->virtual_address = 0x40000000;
  free->length = (0x80000000 - 0x40000000) / 4096;
  free->attributes = MR_ATTR_PRIO_TOP;
  free->type = MR_TYPE_FREE;
  free->parent = as;
  free->next = as->first->next;
  as->first->next = free;
  

  
  free = (memory_region *)allocate_from_slab(regions_slab);
  free->virtual_address = 0x80000000;
  free->length = (0xC0000000 - 0x80000000) / 4096;
  free->attributes = 0;
  free->type = MR_TYPE_FREE;
  free->parent = as;
  free->next = as->first->next;
  as->first->next = free;
  

  

 return as;

}

int expand_region(memory_region * mr, int size) {
  int oldsize = mr->length;
  int inc_amount = 1;

  if (size < 0) {
    bootvideo_printf("Expanding downwards!\n");

    if (mr->type != MR_TYPE_STACK) {
      bootvideo_printf ("Trying to downward expand a nonstack area!\n");
      while (1);
    }
    inc_amount = -1;
  }
  int curpage;
  for (curpage = (inc_amount > 0 ? oldsize : oldsize - 1); (inc_amount > 0 ? curpage < oldsize + size : curpage >= oldsize + size); curpage += inc_amount) {
    /*First verify a pde exists*/
    unsigned int cur_pde = (((unsigned)mr->virtual_address / PAGE_SIZE) + curpage) / 1024;
    unsigned int cur_pte = (((unsigned)mr->virtual_address / PAGE_SIZE) + curpage) % 1024;

    memory_region * newrange = determine_memory_region (mr->parent, mr->virtual_address + curpage * 4096);
    
    if (newrange->type != MR_TYPE_FREE) {
      bootvideo_printf ("Start address: %x\n", newrange);
      bootvideo_printf ("Unable to expand! Memory range is blocked off!\n");
      while (1);
    } else {
      if (inc_amount > 0) {
	newrange->length--;
	newrange->virtual_address += 4096;
      } else {
	newrange->length--;
      }
    }
    pte * pt;
  
    if (((mr->parent->virt_cr3)[cur_pde] & PTE_PRESENT_BIT) == 0) {

      pt = (pte*)get_usable_kernel_virtual_page();
      set_pde(mr->parent->virt_cr3 + cur_pde, get_physical_address_from_kernel_virtual((int)pt), 
	      PTE_PRESENT_BIT | PTE_RW_BIT |PTE_US_BIT);
    } else {
      
      pt = (pte*)get_unused_kernel_virtual_page();
      set_pte( &kpt[ (int)pt / (1024 * PAGE_SIZE)][((int)pt / PAGE_SIZE) % 1024],
	       (mr->parent->virt_cr3)[cur_pde] & 0xFFFFF000,
	       (PTE_PRESENT_BIT | PTE_RW_BIT | PTE_US_BIT));
      __asm__("invlpg (%0)" : : "a" (pt));
      
    }
    set_pte (&pt[cur_pte], allocate_page(0) * PAGE_SIZE,
	     PTE_PRESENT_BIT | PTE_RW_BIT | PTE_US_BIT);
    free_kernel_virtual_page((int)pt);
    __asm__("invlpg (%0)" : : "a" (pt));
    

    __asm__("invlpg (%0)" : : "a" (mr->virtual_address + curpage * 4096));
  }	      
  mr->length += size;

  return 0;
}
         
memory_region * determine_memory_region (address_space * as, unsigned long addr) {
  /*Traverse the mappings*/
  memory_region * m; 
  for (m = as->first->next; m != as->last; m = m->next) {
    if (m->type == MR_TYPE_STACK) {
      if ( (addr < m->virtual_address) &&
	   (addr >= m->virtual_address + 
	    (4096 * m->length))) {
	return m;
      }
    } else {
      if ( (addr >= m->virtual_address) &&
	   (addr < m->virtual_address + 
	    (4096 * m->length))) {
	return m;
      }
    }
  }
  return 0;
}

/*!\brief Create a memory region in virtual address space
 *
 * Given an address space, if addr is NULL it will search for a free block
 * of user address space and split it to create a region with the given
 * flags, attributes, and a parameter.  If addr is non-null, the memory
 * region will be created at the given virtual address, with no overlap
 * checking*/
memory_region * create_region (address_space * as,
             unsigned long addr,
			       int length,
			       memory_region_type flags, int attr,
			       int param) {
  /*First we find a free memory region*/
  memory_region * smallest = 0, *current;
  memory_region * new_region;
  if (addr == 0) {
    for (current = as->first->next; current != as->last; current = current->next) {
      if (current->type == MR_TYPE_FREE) {
        if (current->length >= length) {
    if (smallest == 0) {
      smallest = current;
    } else {
      if (current->length <= smallest->length) {
        smallest = current;
      }
    }
        }
      }
    }
    if (smallest == 0) {
      bootvideo_printf ("Unable to find available memory block!\n");
      return 0;
    }
    
    new_region = (memory_region *)allocate_from_slab (regions_slab);
    
    /*Subdivide the block*/
    if (smallest->attributes & MR_ATTR_PRIO_TOP) {
      smallest->length -= length;
      new_region->virtual_address = smallest->virtual_address + (smallest->length * PAGE_SIZE);
    } else {
      new_region->virtual_address = smallest->virtual_address;
      smallest->virtual_address += (length * PAGE_SIZE);
      smallest->length -= length;
    }
  } else {
    new_region = (memory_region *)allocate_from_slab (regions_slab);
    new_region->virtual_address = addr;
  }
  new_region->length = length;
  new_region->type = flags;
  new_region->attributes = attr;
  new_region->parameter = param;
  new_region->parent = as;
  new_region->next = as->first->next;
  as->first->next = new_region;
  
  
  return new_region;
  

}


void context_print_mmap () {
  context_t * c = get_process (get_current_process ());
  
  address_space * as = c->space;

  memory_region * r;
  for (r = as->first->next; r != as->last; r = r->next) {
    bootvideo_printf ("%x    %x    %s\n", r->virtual_address,
		      r->length * 4096 + r->virtual_address,
		      (r->type == MR_TYPE_FREE ?
		       "FREE" : "USED"));
  }


}
