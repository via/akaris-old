#include <i386/gdt.h>

gdt_entry gdt[5 + MAX_CPUS];
gdtr gdtr_entry = { (uint2)(sizeof(gdt) - 1), (uint4)&gdt};

tss tss_list[MAX_CPUS];

void initialize_gdt() {


  int index;
  for (index = 0; index < 5; index++) {
    gdt[index].limit_low = 0;
    gdt[index].base_low = 0;
    gdt[index].base_middle = 0;
    gdt[index].flags_low = 0;
    gdt[index].flags_high = 0;
    gdt[index].base_high = 0;
  }

  set_gdt_entry(1, 0, 0xFFFFFFFF, GDT_SYSTEM | GDT_CODE);
  set_gdt_entry(2, 0, 0xFFFFFFFF, GDT_SYSTEM | GDT_DATA);
  set_gdt_entry(3, 0, 0xFFFFFFFF, GDT_USER   | GDT_CODE);
  set_gdt_entry(4, 0, 0xFFFFFFFF, GDT_USER   | GDT_DATA);

  for (index = 0; index < MAX_CPUS; index++) {
    tss_list[index].esp0 = (uint4)(0x104020 + 0x4000);
    tss_list[index].ss0  = INDEX_TO_DESCRIPTOR(2);
    
    set_gdt_entry(index + 5,(int) &tss_list[index], sizeof(tss) - 1, GDT_TSS);

  }

  load_gdt(&gdtr_entry);
  load_tr (INDEX_TO_DESCRIPTOR(5));
}


void set_gdt_entry(int index,
		   int base,
		   int length,
		   int flags) {

  gdt[index].limit_low = (uint2)(length & 0xFFFF);
  gdt[index].base_low = (uint2)(base & 0xFFFF);
  gdt[index].base_middle = (uint1)( (base >> 16) & 0xFF);
  gdt[index].base_high = (uint1)( (base >> 24) & (0xFF));
  gdt[index].flags_high = (uint1)( (length >> 16) & 0xF);
  gdt[index].flags_high |= (0x80 | 0x40); /*Set Gran. and Operand Size*/

  if (flags & GDT_TSS) {
    gdt[index].flags_high &= 0xF; /*Clear extra bits*/
    gdt[index].flags_low = GDT_FLAGS_PRESENT | 0x9;
    return;
  } 

  if (flags & GDT_SYSTEM) {
    gdt[index].flags_low = GDT_FLAGS_PRESENT | (0 << 5);
  } else {
    gdt[index].flags_low = GDT_FLAGS_PRESENT | (3 << 5);
  }
  if (flags & GDT_CODE) {
    gdt[index].flags_low |= 0x18;
  } else {
    gdt[index].flags_low |= 0x12;
  }
  
}  
      
