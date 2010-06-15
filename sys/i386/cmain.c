/* cmain.c  -- primary initialization for i386 procs */

#include <i386/multiboot.h>
#include <i386/bootvideo.h>
#include <i386/gdt.h>
#include <i386/interrupt.h>
#include <i386/cpuid.h>
#include <i386/physical_memory.h>
#include <i386/context.h>
#include <i386/slab.h>
#include <i386/paging.h>
#include <i386/process.h>
#include <i386/syscall.h>
#include <i386/mailbox.h>

/* address_space_t *A, *B; */

void timer_interrupt (isr_regs * regs);
void load_modules (multiboot_info_t * mb);

void cmain(multiboot_info_t * mb_info, int magic) {

  bootvideo_cls();
  
  if (magic == MULTIBOOT_BOOTLOADER_MAGIC) {
    bootvideo_printf ("Multiboot compliant loader detected\n");
    bootvideo_printf ("Upper memory available: %dMB\n", mb_info->mem_upper / 1024);
  }

  bootvideo_printf ("AkARiS cmain() entry point\n");
  initialize_gdt();
  bootvideo_printf ("Using new GDT\n");
  initialize_interrupts();  
  bootvideo_printf ("Using new IDT\n");
  initialize_memory (mb_info);
  init_slabs();
  initialize_kernel_paging();
  init_address_space_system();
  initialize_scheduler();
  enable_syscall();
  init_mailboxes ();
  /*load_module (mb_info, 0);*/
  /*load_module (mb_info, 1);*/
  load_modules (mb_info);
  link_irq(32, &timer_interrupt);
/*
  set_current_process (2);
  begin_schedule(&(get_process(2)->registers));
 */ 
  while (1);
  
}


void load_modules (multiboot_info_t * mb) {
  unsigned long cur_mod;
  for (cur_mod = 0; cur_mod < mb->mods_count; ++cur_mod) {

    module_t * m = ((module_t*)mb->mods_addr) + cur_mod;

    int p = create_process();
    execve_elf (get_process (p),(void *) m->mod_start, m->mod_end - m->mod_start, (const char *)m->string);
    bootvideo_printf("Created process, pid %d\n", p);
  }
}

void timer_interrupt(isr_regs* regs) {


    schedule(regs);


}
