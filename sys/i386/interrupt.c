#include <i386/interrupt.h>
#include <i386/bootvideo.h>
#include <i386/mailbox.h>

idt_entry interrupt_descriptors[MAX_INTS];
idtr      idt_pointer = {sizeof(interrupt_descriptors) - 1,
			 (uint4) &interrupt_descriptors};


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

  message m;

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
set_idt_entry (idt_entry * entry, uint4 offset) {
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

}

void 
initialize_idt() {
  
  set_idt_entry((interrupt_descriptors + 0), (uint4)&int00);
  set_idt_entry((interrupt_descriptors + 1), (uint4)&int01);
  set_idt_entry((interrupt_descriptors + 2), (uint4)&int02);
  set_idt_entry((interrupt_descriptors + 3), (uint4)&int03);
  set_idt_entry((interrupt_descriptors + 4), (uint4)&int04);
  set_idt_entry((interrupt_descriptors + 5), (uint4)&int05);
  set_idt_entry((interrupt_descriptors + 6), (uint4)&int06);
  set_idt_entry((interrupt_descriptors + 7), (uint4)&int07);
  set_idt_entry((interrupt_descriptors + 8), (uint4)&int08);
  set_idt_entry((interrupt_descriptors + 9), (uint4)&int09);
  set_idt_entry((interrupt_descriptors + 10), (uint4)&int10);
  set_idt_entry((interrupt_descriptors + 11), (uint4)&int11);
  set_idt_entry((interrupt_descriptors + 12), (uint4)&int12);
  set_idt_entry((interrupt_descriptors + 13), (uint4)&int13);
  set_idt_entry((interrupt_descriptors + 14), (uint4)&int14);
  set_idt_entry((interrupt_descriptors + 15), (uint4)&int15);
  set_idt_entry((interrupt_descriptors + 16), (uint4)&int16);
  set_idt_entry((interrupt_descriptors + 17), (uint4)&int17);
  set_idt_entry((interrupt_descriptors + 18), (uint4)&int18);
  set_idt_entry((interrupt_descriptors + 19), (uint4)&int19);
  set_idt_entry((interrupt_descriptors + 20), (uint4)&int20);
  set_idt_entry((interrupt_descriptors + 21), (uint4)&int21);
  set_idt_entry((interrupt_descriptors + 22), (uint4)&int22);
  set_idt_entry((interrupt_descriptors + 23), (uint4)&int23);
  set_idt_entry((interrupt_descriptors + 24), (uint4)&int24);
  set_idt_entry((interrupt_descriptors + 25), (uint4)&int25);
  set_idt_entry((interrupt_descriptors + 26), (uint4)&int26);
  set_idt_entry((interrupt_descriptors + 27), (uint4)&int27);
  set_idt_entry((interrupt_descriptors + 28), (uint4)&int28);
  set_idt_entry((interrupt_descriptors + 29), (uint4)&int29);
  set_idt_entry((interrupt_descriptors + 30), (uint4)&int30);
  set_idt_entry((interrupt_descriptors + 31), (uint4)&int31);
  set_idt_entry((interrupt_descriptors + 32), (uint4)&int32);
  set_idt_entry((interrupt_descriptors + 33), (uint4)&int33);
  set_idt_entry((interrupt_descriptors + 34), (uint4)&int34);
  set_idt_entry((interrupt_descriptors + 35), (uint4)&int35);
  set_idt_entry((interrupt_descriptors + 36), (uint4)&int36);
  set_idt_entry((interrupt_descriptors + 37), (uint4)&int37);
  set_idt_entry((interrupt_descriptors + 38), (uint4)&int38);
  set_idt_entry((interrupt_descriptors + 39), (uint4)&int39);
  set_idt_entry((interrupt_descriptors + 40), (uint4)&int40);
  set_idt_entry((interrupt_descriptors + 41), (uint4)&int41);
  set_idt_entry((interrupt_descriptors + 42), (uint4)&int42);
  set_idt_entry((interrupt_descriptors + 43), (uint4)&int43);
  set_idt_entry((interrupt_descriptors + 44), (uint4)&int44);
  set_idt_entry((interrupt_descriptors + 45), (uint4)&int45);
  set_idt_entry((interrupt_descriptors + 46), (uint4)&int46);
  set_idt_entry((interrupt_descriptors + 47), (uint4)&int47);
  set_idt_entry((interrupt_descriptors + 0x80), (uint4)&intx80);
  load_idt(&idt_pointer);

}
