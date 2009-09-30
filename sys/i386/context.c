/*context.c*/
#include <i386/context.h>


slab_entry * address_slab;
slab_entry * regions_slab;

memory_region* first;
memory_region* last;

pte ** kpt;

void init_address_space_system() {

  address_slab = create_slab(sizeof(address_space));
  regions_slab = create_slab(sizeof(memory_region));

  first = (memory_region*)allocate_from_slab(regions_slab);
  last = (memory_region*)allocate_from_slab (regions_slab);
  
  first->type = MR_TYPE_SENTINAL;
  first->next = last;
  last->type  = MR_TYPE_SENTINAL;
  last->next = (memory_region *) 0;

  kpt = get_kernel_page_tables();
}

address_space * create_address_space() {

  int c;
  address_space * as = (address_space *)allocate_from_slab(address_slab);
  
  memory_region * core;
  memory_region * stack;

  pde * pages;

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
  as->mappings = last;

  core->virtual_address = 0x40000000;
  core->length = 0;
  core->type = MR_TYPE_CORE;
  core->attributes = 0;
  core->parent = as;
  core->next = first->next;

  stack->virtual_address = 0xC0000000;
  stack->length = 0;
  stack->type = MR_TYPE_STACK;
  stack->attributes = 0;
  stack->parent = as;
  stack->next = core;
  
  first->next = stack;

 return as;

}

int expand_region(memory_region * mr, int size) {
  int oldsize = mr->length;
  int inc_amount = 1;

  if (size < 0) {
    bootvideo_printf("Expanding downwards!\n");
    inc_amount = -1;
  }
  int curpage;
  for (curpage = oldsize; curpage < size; curpage += inc_amount) {
    /*First verify a pde exists*/
    unsigned int cur_pde = (((unsigned)mr->virtual_address / PAGE_SIZE) + curpage) / 1024;
    unsigned int cur_pte = (((unsigned)mr->virtual_address / PAGE_SIZE) + curpage) % 1024;

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
  return 0;
}
         
