/*context.c*/
#include <config.h>
#include <common/types.h>
#include <i386/interrupt.h>
#include <i386/physical_memory.h>
#include <i386/multiboot.h>
#include <common/process.h>
#include <i386/paging.h>
#include <i386/bootvideo.h>
#include <common/slab.h>
#include <i386/context.h>

slab_entry_t * address_slab;
slab_entry_t * regions_slab;


pte ** kpt;

void init_address_space_system() {

  address_slab = create_slab (sizeof (address_space_t));
  regions_slab = create_slab (sizeof (memory_region_t));

  
  kpt = get_kernel_page_tables();
}

address_space_t * create_address_space() {

  int c;
  address_space_t * as = (address_space_t *)allocate_from_slab(address_slab);
  memory_region_t * stack;
  memory_region_t * free;


  pde * pages;


  as->first = (memory_region_t *)allocate_from_slab(regions_slab);
  as->last = (memory_region_t *)allocate_from_slab (regions_slab);
  
  as->first->type = MR_TYPE_SENTINEL;
  as->first->next = as->last;
  as->last->type  = MR_TYPE_SENTINEL;
  as->last->next = (memory_region_t *) 0;


  stack= (memory_region_t *)allocate_from_slab(regions_slab);
 
  pages = (pde*)get_usable_kernel_virtual_page();
  
  memset ( (int*)pages, 0, PAGE_SIZE);
  /*Link in the kernel page directories*/
  
  for (c = 0; c < NUM_KERNEL_PDES; c++) {
    set_pde(pages + c, (int)kpt[c], PTE_PRESENT_BIT | PTE_RW_BIT |
	    PTE_GLOBAL_BIT);
  }

  as->cr3 = (pde*)get_physical_address_from_kernel_virtual((int)pages);
  as->virt_cr3 = pages;
  as->stack = stack;
  as->first->next = stack;

  stack->virtual_address = 0xB0000000;
  stack->length = 0x10000;
  stack->type = MR_TYPE_STACK;
  stack->attributes = 0;
  stack->parent = as;
  stack->next = as->last;
  stack->sharedmem_next = stack;
#if 0 
  free = (memory_region_t *)allocate_from_slab(regions_slab);
  free->virtual_address = (4096 * 1024) * NUM_KERNEL_PDES;
  free->length = (0x40000000 - ((4096 * 1024) * NUM_KERNEL_PDES)) / 4096;
  free->attributes = 0;
  free->type = MR_TYPE_FREE;
  free->parent = as;
  free->next = stack;
  free = (memory_region_t *)allocate_from_slab(regions_slab);
  free->virtual_address = 0x40000000;
  free->length = (0x80000000 - 0x40000000) / 4096;
  free->attributes = MR_ATTR_PRIO_TOP;
  free->type = MR_TYPE_FREE;
  free->parent = as;
  free->next = stack;
  free->sharedmem_next = free;
  as->first->next = free;
#endif

  
  free = (memory_region_t *)allocate_from_slab(regions_slab);
  free->virtual_address = 0x40000000;
  free->length = (0xB0000000 - 0x40000000) / 4096;
  free->attributes = 0;
  free->type = MR_TYPE_FREE;
  free->parent = as;
  free->sharedmem_next = free;
  free->next = as->first->next;
  as->first->next = free;
  

  

 return as;

}

int expand_region(memory_region_t * mr, int size) {
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

    memory_region_t * newrange = determine_memory_region (mr->parent, mr->virtual_address + curpage * 4096);
    
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
         
memory_region_t * determine_memory_region (address_space_t * as, unsigned long addr) {
  /*Traverse the mappings*/
  memory_region_t * m; 
  for (m = as->first->next; m != as->last; m = m->next) {
    if ( (addr >= m->virtual_address) &&
       (addr < m->virtual_address + 
        (4096 * m->length))) {
      return m;
    }
  }
  return 0;
}

/*!\brief Create a memory region in virtual address space
 *
 * Given an address space, if addr is NULL it will search for a free block
 * of user address space and split it to create a region with the given
 * flags, attributes, and a parameter.  If addr is non-null, the memory region
 * will be created at the given virtual address, with any half-overlapping
 * regions being split, completely overlapped regions being deleted, and larger
 * regions being split into three */
memory_region_t * create_region (address_space_t * as,
             unsigned long addr,
			       int length,
			       memory_region_type flags, int attr,
			       unsigned long param) {
  /*First we find a free memory region*/
  memory_region_t * smallest = 0, *current;
  memory_region_t * new_region;
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
    new_region = (memory_region_t *)allocate_from_slab (regions_slab);
    
    /*Subdivide the block*/
  smallest->length -= length;
  new_region->virtual_address = smallest->virtual_address + (smallest->length * PAGE_SIZE);
  
  } else { /* This is the code that executes when the desired virtual address is given*/
    new_region = (memory_region_t *)allocate_from_slab (regions_slab);
    new_region->virtual_address = addr;
    unsigned long stop = addr + length * PAGE_SIZE;                             
    for (current = as->first->next; current != as->last; current = current->next) {
      unsigned long cur_stop = current->virtual_address + current->length * PAGE_SIZE;
      /*Case 1: Complete overlap*/
      if ( (current->virtual_address >= addr) && (cur_stop <= stop)) {
        memory_region_t *newnext = current->next;
        delete_region (current);
        current = newnext;
        /*Case 2: right part of current overlaps region*/
      } else if ( (current->virtual_address <= addr) && (cur_stop <= stop) &&
          (cur_stop > addr)) {
        current->length -= (cur_stop - addr) / PAGE_SIZE;
        /*Case 3: Left part of current overlaps region*/
      } else if ( (cur_stop > stop) && (current->virtual_address >= addr) &&
          (current->virtual_address < stop)) {
        current->length -= (stop - current->virtual_address) / PAGE_SIZE;
        current->virtual_address = stop;
        /*Case 4: Current region overlaps on both sides*/
      } else if ( (current->virtual_address < addr) && (cur_stop > stop) ) {
        memory_region_t *newsplit = (memory_region_t*)allocate_from_slab (regions_slab);
        current->length = (addr - current->virtual_address) / PAGE_SIZE;
        newsplit->attributes = current->attributes;
        newsplit->type = current->type;
        newsplit->parameter = current->parameter;
        newsplit->parent = current->parent;
        newsplit->virtual_address = stop;
        newsplit->length = (cur_stop - stop) / PAGE_SIZE;
        newsplit->next = current->next;
        current->next = newsplit;
      }
    }

  }
  new_region->length = length;
  new_region->type = flags;
  new_region->attributes = attr;
  new_region->parameter = param;
  new_region->parent = as;
  new_region->sharedmem_next = new_region;
  new_region->next = as->first->next;
  as->first->next = new_region;
  return new_region;
  

}

void delete_region (memory_region_t * mr) {
  /*In the future, a memory region can have multiple address spaces, and that
   * will need to be taken into account. for now, the pages are just deleted */
  unsigned long cur_address;
  for (cur_address = mr->virtual_address;
       cur_address < mr->virtual_address + mr->length * PAGE_SIZE;
       cur_address += PAGE_SIZE) {
    int cur_pde = (cur_address / PAGE_SIZE) / 1024;
    int cur_pte = (cur_address / PAGE_SIZE) % 1024;

    pte * cur_pd = (pte *) (mr->parent->virt_cr3[cur_pde] & 0xFFFFF000);
    cur_pd = (pte *)get_mapped_kernel_virtual_page ((unsigned long) cur_pd);
   
    if (mr->sharedmem_next == mr) { /* Not shared at all*/
      deallocate_page (0, cur_pd[cur_pte] & 0xFFFFF000);
    }
    cur_pd[cur_pte] = 0; 
    free_kernel_virtual_page ((unsigned long)cur_pd);
  }

  /* If was a shared page, remove it from the list*/
  if (mr->sharedmem_next != mr) {
    memory_region_t *mr2 = mr->sharedmem_next;
    while (mr2->sharedmem_next != mr) mr2 = mr2->sharedmem_next;
    mr2->sharedmem_next = mr->sharedmem_next;
  }                              
  /*Attempt merge with earlier region*/ 
  memory_region_t *cur = determine_memory_region (mr->parent, mr->virtual_address - 1);
  if (cur->type == MR_TYPE_FREE) {
    cur->length += mr->length;
    memory_region_t *counter = mr->parent->first;
    for (; counter->next != mr; counter = counter->next);
    counter->next = counter->next->next;
    deallocate_from_slab (regions_slab, mr);
    mr = cur;
  }
  cur = determine_memory_region (mr->parent, mr->virtual_address + PAGE_SIZE * mr->length + 1);
  if (cur->type == MR_TYPE_FREE) {
    mr->length += cur->length;
    memory_region_t *counter = mr->parent->first;
    for (; counter->next != cur; counter = counter->next);
    counter->next = counter->next->next;
    deallocate_from_slab (regions_slab, cur);
  }

  mr->type = MR_TYPE_FREE;
   
   

}

void context_print_mmap (memory_region_t *head) {
  
  memory_region_t * r;
  for (r = head; r->type != MR_TYPE_SENTINEL  ; r = r->next) {
    bootvideo_printf ("%x    %x    %s\n", r->virtual_address,
		      r->length * 4096 + r->virtual_address,
		      (r->type == MR_TYPE_FREE ?
		       "FREE" : "USED"));
  }


}

/*! \brief Creates a copy of a memory region in another address space
 *
 * \param destspace Address space to create the new region in
 *
 * \param region Region to copy
 *
 * \param p Set to 1 to preserve current virtual start point
 */
memory_region_t *
clone_region (address_space_t * destspace, memory_region_t *region, int p) {

  memory_region_t *mr = create_region (destspace, 0, region->length, region->type, region->attributes, region->parameter);
  mr->sharedmem_next = region->sharedmem_next;
  region->sharedmem_next = mr;
  unsigned long virt;
  for (virt = mr->virtual_address; virt < mr->virtual_address + mr->length * PAGE_SIZE; virt += PAGE_SIZE) {
    map_user_address (destspace->virt_cr3, virt, user_address_to_physical (region->parent, virt + (region->virtual_address - mr->virtual_address)), p);
    /*TODO: FIX FLAGS*/
  }
  return mr;
}

