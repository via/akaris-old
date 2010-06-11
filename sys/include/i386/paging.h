#ifndef I386_PAGING_H
#define I386_PAGING_H


#include <config.h>
#include <mutex.h>
#include <i386/bootvideo.h>
#include <i386/interrupt.h>
#include <i386/physical_memory.h>


/*PDE/PTE flag defines*/
#define PTE_PRESENT_BIT (1 << 0)
#define PTE_RW_BIT      (1 << 1)
#define PTE_US_BIT      (1 << 2)
#define PTE_PWT_BIT     (1 << 3)
#define PTE_PCD_BIT     (1 << 4)
#define PTE_ACCESS_BIT  (1 << 5)
#define PTE_DIRTY_BIT   (1 << 6)
#define PTE_PS_BIT      (1 << 7)
#define PTE_GLOBAL_BIT  (1 << 8)
#define PTE_KUSED_BIT   (1 << 9)

#define NUM_KERNEL_PDES 16

typedef int pte;
typedef int pde;

void initialize_kernel_paging();
pte ** get_kernel_page_tables();

struct context;

void set_pte(pte *, int address, int flags);
void set_pde(pde *, int address, int flags);

void enable_paging();
void set_cr3(pde *);
pde* get_cr3();

int get_physical_address_from_kernel_virtual(int);

int get_unused_kernel_virtual_page();
int get_usable_kernel_virtual_page();
unsigned long get_mapped_kernel_virtual_page (unsigned long addr);
unsigned long map_user_virtual_to_kernel(struct context *,
					 unsigned long);
void free_kernel_virtual_page(int);
  
void map_user_region_to_physical (memory_region * mr, unsigned long phys_addr); 
void page_fault_handler(isr_regs *);


#endif
