#include <i386/types.h>
#include <config.h>
#include <i386/interrupt.h>
#include <i386/bootvideo.h>
#include <i386/mailbox.h>
#include <i386/pic.h>

idt_entry interrupt_descriptors[MAX_INTS];
idtr      idt_pointer = {sizeof(interrupt_descriptors) - 1,
			 (uint32) &interrupt_descriptors};
int kernel_reenter;

void (*int_handler[256])(isr_regs*);
int int_to_pid[256];

void 
link_irq(int irq, void (*func)(isr_regs*)) {
  int_handler[irq] = func;
}

void
link_irq_to_pid(int irq, int pid) {

  int_to_pid[irq] = pid;

}

isr_regs* c_isr(isr_regs* regs_in) {

  message_t m;

  if (int_handler[regs_in->int_no] != 0) {
    int_handler[regs_in->int_no](regs_in);
  }

  if (int_to_pid[regs_in->int_no]) {
    m.src_pid = -1;
    m.dest_pid = int_to_pid[regs_in->int_no];
    send_message (&m, 1);
  }

  /*acknowledge interrupts*/
  if (regs_in->int_no >= 40) {
        outportb(0xA0, 0x20);
  }

  outportb(0x20, 0x20);


  return regs_in;
}


void 
set_idt_entry (idt_entry * entry, uint32 offset) {
  entry->offset_low  = offset & 0xFFFF;
  entry->offset_high = (offset & 0xFFFF0000) >> 16;
  entry->zero = 0;
  entry->selector = 0x08; /*System Code Segment*/
  entry->type = IDT_PRESENT | IDT_INTGATE | (3 << 5);
}

void 
initialize_interrupts() {
  initialize_idt();
  int a;
  for (a = 0; a < 256; a++) {
    int_handler[a] = 0;
    int_to_pid[a] = 0;
  }
  if (picAvailable()) {
    picRemapIRQs();
  }

  kernel_reenter = 0;

}

void 
initialize_idt() {
  
  set_idt_entry((interrupt_descriptors + 0), (uint32)&int00);
  set_idt_entry((interrupt_descriptors + 1), (uint32)&int01);
  set_idt_entry((interrupt_descriptors + 2), (uint32)&int02);
  set_idt_entry((interrupt_descriptors + 3), (uint32)&int03);
  set_idt_entry((interrupt_descriptors + 4), (uint32)&int04);
  set_idt_entry((interrupt_descriptors + 5), (uint32)&int05);
  set_idt_entry((interrupt_descriptors + 6), (uint32)&int06);
  set_idt_entry((interrupt_descriptors + 7), (uint32)&int07);
  set_idt_entry((interrupt_descriptors + 8), (uint32)&int08);
  set_idt_entry((interrupt_descriptors + 9), (uint32)&int09);
  set_idt_entry((interrupt_descriptors + 10), (uint32)&int10);
  set_idt_entry((interrupt_descriptors + 11), (uint32)&int11);
  set_idt_entry((interrupt_descriptors + 12), (uint32)&int12);
  set_idt_entry((interrupt_descriptors + 13), (uint32)&int13);
  set_idt_entry((interrupt_descriptors + 14), (uint32)&int14);
  set_idt_entry((interrupt_descriptors + 15), (uint32)&int15);
  set_idt_entry((interrupt_descriptors + 16), (uint32)&int16);
  set_idt_entry((interrupt_descriptors + 17), (uint32)&int17);
  set_idt_entry((interrupt_descriptors + 18), (uint32)&int18);
  set_idt_entry((interrupt_descriptors + 19), (uint32)&int19);
  set_idt_entry((interrupt_descriptors + 20), (uint32)&int20);
  set_idt_entry((interrupt_descriptors + 21), (uint32)&int21);
  set_idt_entry((interrupt_descriptors + 22), (uint32)&int22);
  set_idt_entry((interrupt_descriptors + 23), (uint32)&int23);
  set_idt_entry((interrupt_descriptors + 24), (uint32)&int24);
  set_idt_entry((interrupt_descriptors + 25), (uint32)&int25);
  set_idt_entry((interrupt_descriptors + 26), (uint32)&int26);
  set_idt_entry((interrupt_descriptors + 27), (uint32)&int27);
  set_idt_entry((interrupt_descriptors + 28), (uint32)&int28);
  set_idt_entry((interrupt_descriptors + 29), (uint32)&int29);
  set_idt_entry((interrupt_descriptors + 30), (uint32)&int30);
  set_idt_entry((interrupt_descriptors + 31), (uint32)&int31);
  set_idt_entry((interrupt_descriptors + 32), (uint32)&int32);
  set_idt_entry((interrupt_descriptors + 33), (uint32)&int33);
  set_idt_entry((interrupt_descriptors + 34), (uint32)&int34);
  set_idt_entry((interrupt_descriptors + 35), (uint32)&int35);
  set_idt_entry((interrupt_descriptors + 36), (uint32)&int36);
  set_idt_entry((interrupt_descriptors + 37), (uint32)&int37);
  set_idt_entry((interrupt_descriptors + 38), (uint32)&int38);
  set_idt_entry((interrupt_descriptors + 39), (uint32)&int39);
  set_idt_entry((interrupt_descriptors + 40), (uint32)&int40);
  set_idt_entry((interrupt_descriptors + 41), (uint32)&int41);
  set_idt_entry((interrupt_descriptors + 42), (uint32)&int42);
  set_idt_entry((interrupt_descriptors + 43), (uint32)&int43);
  set_idt_entry((interrupt_descriptors + 44), (uint32)&int44);
  set_idt_entry((interrupt_descriptors + 45), (uint32)&int45);
  set_idt_entry((interrupt_descriptors + 46), (uint32)&int46);
  set_idt_entry((interrupt_descriptors + 47), (uint32)&int47);
  set_idt_entry((interrupt_descriptors + 0x80), (uint32)&intx80);
  load_idt(&idt_pointer);

}
