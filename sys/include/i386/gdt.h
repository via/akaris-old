#ifndef I386_GDT_H
#define I386_GDT_H

#include <i386/types.h>
#include <config.h>

#define GDT_SYSTEM 0x1
#define GDT_USER   0x2
#define GDT_CODE   0x4
#define GDT_DATA   0x8
#define GDT_TSS    0x10

#define GDT_FLAGS_PRESENT 0x80

#define INDEX_TO_DESCRIPTOR(x) ((x) << 3)

extern uint4 * stack;

typedef struct {
  uint2 limit_low;
  uint2 base_low;
  uint1 base_middle;
  uint1 flags_low;
  uint1 flags_high;
  uint1 base_high;
} __attribute__((__packed__)) gdt_entry;

typedef struct {
  uint2 length;
  uint4 base;
} __attribute__((__packed__)) gdtr;

typedef struct {
  uint2 link;
  uint2 link_h;

  uint4 esp0;
  uint2 ss0;
  uint2 ss0_h;

  uint4 esp1;
  uint2 ss1;
  uint2 ss1_h;

  uint4 esp2;
  uint2 ss2;
  uint2 ss2_h;

  uint4 cr3;
  uint4 eip;
  uint4 eflags;

  uint4 eax;
  uint4 ecx;
  uint4 edx;
  uint4 ebx;

  uint4 esp;
  uint4 ebp;

  uint4 esi;
  uint4 edi;

  uint2 es;
  uint2 es_h;

  uint2 cs;
  uint2 cs_h;

  uint2 ss;
  uint2 ss_h;

  uint2 ds;
  uint2 ds_h;

  uint2 fs;
  uint2 fs_h;

  uint2 gs;
  uint2 gs_h;

  uint2 ldt;
  uint2 ldt_h;

  uint2 trap;
  uint2 iomap;
} __attribute__((__packed__)) tss;

void initialize_gdt();
extern void load_gdt (gdtr*);
extern void load_tr  (uint2);
void set_kernel_tss_stack (void *);
void set_gdt_entry(int index,
		   int base,
		   int length,
		   int flags);


#endif
