/*paging.c*/
#include <i386/types.h>
#include <config.h>
#include <i386/physical_memory.h>
#include <i386/bootvideo.h>
#include <i386/process.h>
#include <i386/paging.h>
#include <i386/context.h>
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
  
  memory_region_t * mr = determine_memory_region (context->space, (unsigned long) regs->eip);

  if (mr == context->space->stack) {
    bootvideo_printf ("Page fault in stack of pid %d!\n", context->pid);
    while (1); /*Kill task*/
  } else if (mr->type == MR_TYPE_CORE ) {
    bootvideo_printf ("Page fault in core of pid %d!\n", context->pid);
    
    if (determine_memory_region (context->space, (unsigned long) address) == context->space->stack) {
      map_user_address (context->space->virt_cr3, address & 0xFFFFF000, allocate_page (0) * PAGE_SIZE, 0);
      bootvideo_printf ("Added space to swap! @ %x\n", address & 0xFFFFF000);
    }
  }
  /*Delay loop to easily see page fault info*/

  

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
						     
  
void free_kernel_virtual_page (unsigned long addr) {
  addr /= 4096;
  int cur_pde = addr / 1024;
  int cur_pte = addr % 1024;

  kernel_page_tables[cur_pde][cur_pte] = 0;
}

/*! \brief Unallocates the physical page that virtaddr points to*/
void free_kernel_physical_page (unsigned long virtaddr) {
  virtaddr /= PAGE_SIZE;
  int cur_pde = virtaddr / 1024;
  int cur_pte = virtaddr % 1024;
  if (virtaddr * PAGE_SIZE < 0x40000000) {
    set_page_status (0, kernel_page_tables[cur_pde][cur_pte] & 0xFFFFF000, 0);
  }
}
/*! \brief Map a userspace memory region to physical addresses.
 * 
 * Given a memory region, maps to a given physical address.  If phys_addr is
 * NULL, will allocate physical memory (useful for anonymous mmap).  Note this
 * function must be called from within the context of the process to be
 * modified.
 */
void map_user_region_to_physical ( memory_region_t * mr, unsigned long phys_addr) {
  unsigned long cur_virt = mr->virtual_address;
  pde * cur_pd = mr->parent->virt_cr3;
  if ((cur_virt & 0xFFF) != 0) {/*If it doesn't start on a page boundary...*/ 
    return;
  }

  unsigned long cur_phys = phys_addr;

  for (; cur_virt < mr->virtual_address + mr->length * PAGE_SIZE; cur_virt += PAGE_SIZE) {
    unsigned long cur_pde = (cur_virt / PAGE_SIZE) / 1024;
    unsigned long cur_pte = (cur_virt / PAGE_SIZE) % 1024;
    pte * cur_pde_virt;
    if (phys_addr == 0) {
      cur_phys =  allocate_page (0) * PAGE_SIZE;
    }
    
    if ((cur_pd[cur_pde] & PTE_PRESENT_BIT) == 0) {
      set_pde ( &cur_pd[cur_pde], allocate_page (0) * PAGE_SIZE, PTE_PRESENT_BIT | 
          PTE_RW_BIT | PTE_US_BIT);
    }    
    cur_pde_virt = (pte *) get_mapped_kernel_virtual_page (cur_pd[cur_pde] & 0xFFFFF000);
    set_pte (&cur_pde_virt[cur_pte], cur_phys, PTE_PRESENT_BIT | PTE_US_BIT | PTE_RW_BIT);
    __asm__("invlpg (%0)" : : "a" (cur_pd[cur_pde] & 0xFFFFF000));
    __asm__("invlpg (%0)" : : "a" (cur_pde_virt[cur_pte] & 0xFFFFF000));
    cur_phys += PAGE_SIZE;
  }

}

void map_user_address (pde * virt_cr3, unsigned long virtaddr, unsigned long physaddr, int flags) {

  unsigned long cur_pde = (virtaddr / PAGE_SIZE) / 1024;
  unsigned long cur_pte = (virtaddr / PAGE_SIZE) % 1024;

  if ((virt_cr3[cur_pde] & PTE_PRESENT_BIT) == 0) {
    set_pde (&virt_cr3[cur_pde], allocate_page (0) * PAGE_SIZE, PTE_PRESENT_BIT | PTE_RW_BIT | PTE_US_BIT);
  }

  pte * pt = (pte *) (virt_cr3[cur_pde] & 0xFFFFF000);

  set_pte (&pt[cur_pte], physaddr, PTE_PRESENT_BIT | PTE_RW_BIT | PTE_US_BIT | flags);
  __asm__("invlpg (%0)" : : "a" (virtaddr));
}

unsigned long user_address_to_physical (address_space_t * as, unsigned long virtaddr) {

  unsigned long cur_pde = (virtaddr / PAGE_SIZE) / 1024;
  unsigned long cur_pte = (virtaddr / PAGE_SIZE) % 1024;
  if (as->virt_cr3[cur_pde] == 0) return 0;
  pte * pt = (pte *) get_mapped_kernel_virtual_page (as->virt_cr3[cur_pde] & 0xFFFFF000);
  unsigned long phys = pt[cur_pte] & 0xFFFFF000;
  free_kernel_virtual_page ((unsigned long)pt);
  return phys;
}
