#ifndef I386_INTERRUPT_H
#define I386_INTERRUPT_H

#include <i386/types.h>
#include <i386/pic.h>

#define IDT_PRESENT 0x80
#define IDT_INTGATE 0xE

#define MAX_INTS 0x81

typedef struct  {
  uint4 gs, fs, es, ds;      
  uint4 edi, esi, ebp, esp, ebx, edx, ecx, eax;
  uint4 int_no, err_code;
  uint4 eip, cs, eflags, useresp, ss;
} __attribute__((__packed__)) isr_regs;

typedef struct {
  uint2 offset_low;
  uint2 selector;
  uint1 zero;
  uint1 type;
  uint2 offset_high;
} __attribute__((__packed__)) idt_entry;

typedef struct idtr {
  uint2 length;
  uint4 location;
} __attribute__((__packed__)) idtr;

void set_idt_entry(idt_entry*, uint4 offset);
void initialize_idt();
void initialize_interrupts();
void load_idt(idtr*);
void link_irq(int, void(*)(isr_regs*));
/*Individual addresses for isrs*/
extern void (*int00)();
extern void (*int01)();
extern void (*int02)();
extern void (*int03)();
extern void (*int04)();
extern void (*int05)();
extern void (*int06)();
extern void (*int07)();
extern void (*int08)();
extern void (*int09)();
extern void (*int10)();
extern void (*int11)();
extern void (*int12)();
extern void (*int13)();
extern void (*int14)();
extern void (*int15)();
extern void (*int16)();
extern void (*int17)();
extern void (*int18)();
extern void (*int19)();
extern void (*int20)();
extern void (*int21)();
extern void (*int22)();
extern void (*int23)();
extern void (*int24)();
extern void (*int25)();
extern void (*int26)();
extern void (*int27)();
extern void (*int28)();
extern void (*int29)();
extern void (*int30)();
extern void (*int31)();
extern void (*int32)();
extern void (*int33)();
extern void (*int34)();
extern void (*int35)();
extern void (*int36)();
extern void (*int37)();
extern void (*int38)();
extern void (*int39)();
extern void (*int40)();
extern void (*int41)();
extern void (*int42)();
extern void (*int43)();
extern void (*int44)();
extern void (*int44)();
extern void (*int45)();
extern void (*int46)();
extern void (*int47)();
extern void (*intx80)();


#endif
