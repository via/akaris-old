#ifndef I386_GDT_H
#define I386_GDT_H

#include <i386/types.h>

#define GDT_SYSTEM 0x1
#define GDT_USER   0x2
#define GDT_CODE   0x4
#define GDT_DATA   0x8
#define GDT_TSS    0x10

#define GDT_FLAGS_PRESENT 0x80

#define INDEX_TO_DESCRIPTOR(x) ((x) << 3)

extern uint32 * stack;

typedef struct {
  uint16 limit_low;
  uint16 base_low;
  uint8 base_middle;
  uint8 flags_low;
  uint8 flags_high;
  uint8 base_high;
} __attribute__((__packed__)) gdt_entry;

typedef struct {
  uint16 length;
  uint32 base;
} __attribute__((__packed__)) gdtr;

typedef struct {
  uint16 link;
  uint16 link_h;

  uint32 esp0;
  uint16 ss0;
  uint16 ss0_h;

  uint32 esp1;
  uint16 ss1;
  uint16 ss1_h;

  uint32 esp2;
  uint16 ss2;
  uint16 ss2_h;

  uint32 cr3;
  uint32 eip;
  uint32 eflags;

  uint32 eax;
  uint32 ecx;
  uint32 edx;
  uint32 ebx;

  uint32 esp;
  uint32 ebp;

  uint32 esi;
  uint32 edi;

  uint16 es;
  uint16 es_h;

  uint16 cs;
  uint16 cs_h;

  uint16 ss;
  uint16 ss_h;

  uint16 ds;
  uint16 ds_h;

  uint16 fs;
  uint16 fs_h;

  uint16 gs;
  uint16 gs_h;

  uint16 ldt;
  uint16 ldt_h;

  uint16 trap;
  uint16 iomap;
} __attribute__((__packed__)) tss;

void initialize_gdt();
extern void load_gdt (gdtr*);
extern void load_tr  (uint16);
void set_kernel_tss_stack (void *);
void set_gdt_entry(int index,
		   int base,
		   int length,
		   int flags);


#endif
