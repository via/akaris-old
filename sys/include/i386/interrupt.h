#ifndef I386_INTERRUPT_H
#define I386_INTERRUPT_H

#include <common/types.h>

#define IDT_PRESENT 0x80
#define IDT_INTGATE 0xE

#define MAX_INTS 0x81

extern int kernel_reenter;

typedef struct {
  uint32 gs, fs, es, ds;      
  uint32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
  uint32 int_no, err_code;
  uint32 eip, cs, eflags, useresp, ss;
} __attribute__((__packed__)) isr_regs;

typedef struct {
  uint16 offset_low;
  uint16 selector;
  uint8 zero;
  uint8 type;
  uint16 offset_high;
} __attribute__((__packed__)) idt_entry;

typedef struct idtr {
  uint16 length;
  uint32 location;
} __attribute__((__packed__)) idtr;

void set_idt_entry(idt_entry*, uint32 offset);
void initialize_idt();
void initialize_interrupts();
void load_idt(idtr*);
void link_irq(int, void(*)(isr_regs*));
void link_irq_to_fifo(int, uint32);
/*Individual addresses for isrs*/
extern void (*intr00)();
extern void (*intr01)();
extern void (*intr02)();
extern void (*intr03)();
extern void (*intr04)();
extern void (*intr05)();
extern void (*intr06)();
extern void (*intr07)();
extern void (*intr08)();
extern void (*intr09)();
extern void (*intr10)();
extern void (*intr11)();
extern void (*intr12)();
extern void (*intr13)();
extern void (*intr14)();
extern void (*intr15)();
extern void (*intr16)();
extern void (*intr17)();
extern void (*intr18)();
extern void (*intr19)();
extern void (*intr20)();
extern void (*intr21)();
extern void (*intr22)();
extern void (*intr23)();
extern void (*intr24)();
extern void (*intr25)();
extern void (*intr26)();
extern void (*intr27)();
extern void (*intr28)();
extern void (*intr29)();
extern void (*intr30)();
extern void (*intr31)();
extern void (*intr32)();
extern void (*intr33)();
extern void (*intr34)();
extern void (*intr35)();
extern void (*intr36)();
extern void (*intr37)();
extern void (*intr38)();
extern void (*intr39)();
extern void (*intr40)();
extern void (*intr41)();
extern void (*intr42)();
extern void (*intr43)();
extern void (*intr44)();
extern void (*intr44)();
extern void (*intr45)();
extern void (*intr46)();
extern void (*intr47)();
extern void (*intx80)();


#endif
