#include <config.h>
#include <i386/types.h>
#include <i386/process.h>
#include <i386/slab.h>
#include <i386/context.h>
#include <i386/paging.h>
#include <i386/bootvideo.h>
#include <i386/gdt.h>
#include <elf.h>

  slab_entry_t * context_slab;
  context_t * context_list;

  int next_avail_pid;
  context_t * cur_process;

  void initialize_scheduler() {
    context_slab = create_slab(sizeof(context_t));
    context_list = (context_t *) 0;
    next_avail_pid = 1;
  }
  /*! \brief Replaces current process image with a new one.
   * 
   * Given a current context, replaces all core and stack regions with ones
   * created from an ELF file image in memory, including the creation of a process
   * data section with the given environmental variables*/
  int execve_elf (context_t * cur_process, void * elf_image, unsigned long length, const char * env) {
    bootvideo_printf ("elf image = %l\n", elf_image);
    if ((cur_process == 0) ||
        (elf_image == 0)) return 0;                 
    if (length >= 0x20000000) {
    bootvideo_printf ("Unsupported Length: %x\n", length);
    return -1;
  }
  /*We need to make a fake segment before 0x40000000 and copy the elf image
   * there so we can safely destroy all the process's memory regions */
  memory_region_t temp;

  temp.parent = cur_process->space;
  temp.type = MR_TYPE_KERNEL;
  temp.virtual_address = 0x20000000;
  temp.length = (length + PAGE_SIZE) / PAGE_SIZE;
  map_user_region_to_physical(&temp, 0); /*Make it be backed by mem*/
  memcpy ( (void *)0x20000000, elf_image, length);
  elf_image = (void *) 0x20000000;

  memory_region_t * cur;
  for (cur = cur_process->space->first; cur->next != cur_process->space->last;
      cur = cur->next) {
    if (cur->next->type == MR_TYPE_CORE) {
      delete_region (cur->next);
    }
  }

  Elf32_Ehdr * elf_header = (Elf32_Ehdr *) elf_image;
  Elf32_Phdr * prg_header = (Elf32_Phdr *) ((unsigned long) elf_image + elf_header->e_phoff);
  uint16 cur_phdr;

  /*Verify the header*/
  if ( !(elf_header->e_ident[EI_MAG0] == ELFMAG0) ||
       !(elf_header->e_ident[EI_MAG1] == ELFMAG1) ||
       !(elf_header->e_ident[EI_MAG2] == ELFMAG2) ||
       !(elf_header->e_ident[EI_MAG3] == ELFMAG3) ) {
    bootvideo_printf ("Not passed a valid ELF image\n");
    return -1;
  }

  for (cur_phdr = 0; cur_phdr < elf_header->e_phnum; ++cur_phdr, ++prg_header) {
   if (prg_header->p_type != PT_LOAD) continue;
   memory_region_t * mr = create_region (cur_process->space, prg_header->p_vaddr, ((prg_header->p_memsz + PAGE_SIZE) / PAGE_SIZE), MR_TYPE_CORE, 0, 0);
   map_user_region_to_physical (mr, 0);
   memcpy ( (char *)prg_header->p_vaddr,(char *) ((unsigned long)elf_image + prg_header->p_offset), prg_header->p_filesz);
  }
  cur_process->registers.eip = elf_header->e_entry;
  elf_header->e_ident[0] = *env;
  return 0;
}

  

int create_process() {
  context_t * new_context, *t;
  new_context = (context_t*)allocate_from_slab(context_slab);
  if (context_list == 0) {
    context_list = new_context;
  } else {
    for (t = context_list;
	 t->next != 0;
	 t = t->next); /*Get to end*/
    t->next = new_context;
  }
  new_context->next = 0;
  new_context->registers.eax = 0xDEADBEEF;
  new_context->registers.gs = 0x23;
  new_context->registers.fs = 0x23;
  new_context->registers.es = 0x23;
  new_context->registers.ds = 0x23;
  new_context->registers.ss = 0x23;
  new_context->registers.eip = 0x40000000;
  new_context->registers.cs = 0x1B;
  new_context->registers.eflags = 0x200; /*IF set*/
  new_context->registers.useresp = 0xBFFFFFF0; /*0xC0000FFF;*/

  new_context->space = create_address_space();
  expand_region(new_context->space->stack, -1);
  set_cr3(new_context->space->cr3);

  new_context->pid = next_avail_pid;
  next_avail_pid++;

  new_context->status = PROCESS_STATUS_RUNNING;

  return new_context->pid;
}

context_t * get_process(int pid) {
  if (context_list == 0)
    return (context_t*) 0;

  context_t * c;
  for (c = context_list;
       (c->pid != pid) && (c->next != 0);
       c = c->next);
  if (c->pid != pid) {
    bootvideo_printf("Invalid pid %d\n", pid);
    return 0;
  } else {
    return c;
  }
}



void schedule(isr_regs * regs) {

  int temp_stack[100];

  if (kernel_reenter) {
    return;
  }
  memcpy((char*)&cur_process->registers, (char*)regs, sizeof(isr_regs));

  context_t *orig = cur_process;

  do {
    cur_process = cur_process->next;
    if (cur_process == 0)
      cur_process = context_list;
    if (cur_process == orig) { /*We've looped the whole list*/
      if (cur_process->status != PROCESS_STATUS_RUNNING) {
	kernel_reenter = 1;
	set_kernel_tss_stack ((void*)temp_stack);
	__asm__("sti\n"
		"hlt");
	set_kernel_tss_stack (0);
	__asm__("cli");
	kernel_reenter = 0;

      }
    }
  } while (cur_process->status != PROCESS_STATUS_RUNNING);
  memcpy((char*)regs,(char*) &cur_process->registers, sizeof(isr_regs));
  
  set_cr3(cur_process->space->cr3);
}

int get_current_process() {
  return cur_process->pid;
}

void set_current_process (int pid) {
  cur_process = get_process ( pid );
}
