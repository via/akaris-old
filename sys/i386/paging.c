/*paging.c*/

#include <i386/paging.h>
#include <i386/process.h>

/*Temporary page directory for early kernel usage*/
pde * kernel_pd;

/*Page tables making up a fully pde's worth*/
pte * kernel_page_tables[NUM_KERNEL_PDES];

pte ** get_kernel_page_tables() {
  return kernel_page_tables;
}

void initialize_kernel_paging() {
  int c;

  kernel_pd = (pde *)(allocate_page(0) * PAGE_SIZE);
  memset ( (int*)kernel_pd, 0, PAGE_SIZE);

  for (c = 0; c < NUM_KERNEL_PDES; c++) {
    kernel_page_tables[c] = (pte*)(allocate_page(0) * PAGE_SIZE);
    memset((int*)kernel_page_tables[c], 0, PAGE_SIZE);
    set_pde(&kernel_pd[c], (int)kernel_page_tables[c], 
	    PTE_PRESENT_BIT | PTE_RW_BIT | PTE_GLOBAL_BIT);
  }

  /*This will identity map the first 4 mb*/
  for (c = 0; c < 1024; c++)
    set_pte( &kernel_page_tables[0][c], c * 4096,
	     PTE_PRESENT_BIT | PTE_RW_BIT | PTE_GLOBAL_BIT);
 
  link_irq (14, &page_fault_handler);

  set_cr3 (kernel_pd);
  enable_paging();


}

void set_pte( pte * p, int address, int flags) {
  *p = (pte)((address & 0xFFFFF000) + flags);
}

void set_pde( pde * p, int address, int flags) {
  *p = (pde)((address & 0xFFFFF000) + flags);
}




void set_cr3(pde * pd) {
  if ( ((int)pd & 0xFFF) != 0)
    return;

  __asm__ ("movl %%eax, %%cr3" : : "a" (pd));

}

pde * get_cr3() {
  pde * pd;
  __asm__ ("movl %%cr3, %%eax" : "=a" (pd));
  return pd;
}
void enable_paging() {
  __asm__ ("movl %cr0, %eax\n"
       "orl $0x80000000, %eax\n"
       "movl %eax, %cr0");
}

void page_fault_handler(isr_regs * regs) {

  bootvideo_printf("Page Fault! %x \n", regs->int_no);
  unsigned long address;
  __asm__("movl %%cr2, %0" : "=r" (address));
  bootvideo_printf("Offending address: %x @ %x\n", address, regs->eip);
  bootvideo_printf("Error code: %x\n", regs->err_code);



  /*Check to see if area is a stack region*/ 
  if (!get_current_process ()) {
    bootvideo_printf ("Haven't set up processes yet. hanging\n");
    while (1);
  }
  context_t * context = get_process (get_current_process ());
  
  memory_region * mr = determine_memory_region (context->space, (unsigned long) regs->eip);

  if (mr == context->space->stack) {
    bootvideo_printf ("Page fault in stack of pid %d!\n", context->pid);
    while (1); /*Kill task*/
  } else if (mr == context->space->core) {
    bootvideo_printf ("Page fault in core of pid %d!\n", context->pid);
    if ( (address < context->space->stack->virtual_address +
	  (4096 * context->space->stack->length)) && 
	 (address > context->space->stack->virtual_address + 
	  (4096 * context->space->stack->length) - 4096)) {
      
      bootvideo_printf ("Offending address is below stack!\n");
      expand_region (context->space->stack, -1);

    }
  }
  /*Delay loop to easily see page fault info*/
  dump_slab_info ();
  for (address = 0; address < 200000000; ++address);

  

}

int get_unused_kernel_virtual_page() {
  
  int cur_pde, cur_pte;
  int found = 0;
  for (cur_pde = 1; cur_pde < NUM_KERNEL_PDES; cur_pde++) {
    for (cur_pte = 0; cur_pte < 1024; cur_pte++) {
      if (((int)kernel_page_tables[cur_pde][cur_pte] &
	   PTE_KUSED_BIT) == 0) {
	found = 1;
	break;
      }
    }

    if (found == 1) break;
  }

  kernel_page_tables[cur_pde][cur_pte] |= (PTE_KUSED_BIT | PTE_PRESENT_BIT |
					   PTE_GLOBAL_BIT | PTE_RW_BIT);
  return ((cur_pde * 1024) + cur_pte) * 4096;
}
	
int get_usable_kernel_virtual_page() {
  int virt_addr = get_unused_kernel_virtual_page();
  virt_addr /= PAGE_SIZE;
  int cur_pde = virt_addr / 1024;
  int cur_pte = virt_addr % 1024;

  int phys_addr = allocate_page(0) * PAGE_SIZE;
  if (phys_addr == 0) {
    bootvideo_printf("Out_of_memory!\n");
    while (1);
  }
  kernel_page_tables[cur_pde][cur_pte] |= phys_addr;
  __asm__("invlpg (%0)" : : "a" (virt_addr * PAGE_SIZE));
  return virt_addr * PAGE_SIZE;
}

unsigned long get_mapped_kernel_virtual_page (unsigned long addr) {
  int virt_addr = get_unused_kernel_virtual_page();
  virt_addr /= PAGE_SIZE;
  int cur_pde = virt_addr / 1024;
  int cur_pte = virt_addr % 1024;

 
  kernel_page_tables[cur_pde][cur_pte] |= addr;
  __asm__("invlpg (%0)" : : "a" (virt_addr * PAGE_SIZE));
  return virt_addr * PAGE_SIZE;
}  

int get_physical_address_from_kernel_virtual(int addr) {
  if (addr > NUM_KERNEL_PDES * 4 * 1024 * 1024) 
    return -1;
  addr /= PAGE_SIZE;
  int cur_pde = addr / 1024;
  int cur_pte = addr % 1024;
  
  int phys_addr = kernel_page_tables[cur_pde][cur_pte] & 0xFFFFF000;
  return phys_addr;
}

unsigned long map_user_virtual_to_kernel(context_t * c, unsigned long addr) {
  addr /= 4096;
  int cur_pde = addr / 1024;
  int cur_pte = addr % 1024;
  
  pte * list = (pte *) get_mapped_kernel_virtual_page(c->space->virt_cr3[cur_pde]);
  unsigned long p_addr = list[cur_pte] & 0xFFFFF000;
  free_kernel_virtual_page((int)list);
  return get_mapped_kernel_virtual_page (p_addr);
}
						     
  
void free_kernel_virtual_page (int addr) {
  addr /= 4096;
  int cur_pde = addr / 1024;
  int cur_pte = addr % 1024;

  kernel_page_tables[cur_pde][cur_pte] = 0;
}
