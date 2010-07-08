#include <i386/types.h>
#include <config.h>
#include <i386/interrupt.h>
#include <i386/bootvideo.h>
#include <i386/kfifo.h>
#include <i386/pic.h>

idt_entry interrupt_descriptors[MAX_INTS];
idtr      idt_pointer = {sizeof(interrupt_descriptors) - 1,
			 (uint32) &interrupt_descriptors};
int kernel_reenter;

void (*int_handler[256])(isr_regs*);
uint32 int_to_pid[256];

void 
link_irq(int irq, void (*func)(isr_regs*)) {
  int_handler[irq] = func;
}

void
link_irq_to_fifo (int irq, uint32 fifo) {

  int_to_pid[irq] = fifo;

}

isr_regs* c_isr(isr_regs* regs_in) {
  kfifo_error e;

  if (int_handler[regs_in->int_no] != 0) {
    int_handler[regs_in->int_no](regs_in);
  }

  /*Send a blank byte on the fifo to make it ready*/
  if (int_to_pid[regs_in->int_no] != 0) {
    e = kfifo_write_fifo (int_to_pid[regs_in->int_no], kfifo_kernel_pid, "\0", 1); 
    if (e != KFIFO_SUCCESS) {
      bootvideo_printf ("FAILED to send\n");
    }
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
  
  set_idt_entry((interrupt_descriptors + 0), (uint32)&intr00);
  set_idt_entry((interrupt_descriptors + 1), (uint32)&intr01);
  set_idt_entry((interrupt_descriptors + 2), (uint32)&intr02);
  set_idt_entry((interrupt_descriptors + 3), (uint32)&intr03);
  set_idt_entry((interrupt_descriptors + 4), (uint32)&intr04);
  set_idt_entry((interrupt_descriptors + 5), (uint32)&intr05);
  set_idt_entry((interrupt_descriptors + 6), (uint32)&intr06);
  set_idt_entry((interrupt_descriptors + 7), (uint32)&intr07);
  set_idt_entry((interrupt_descriptors + 8), (uint32)&intr08);
  set_idt_entry((interrupt_descriptors + 9), (uint32)&intr09);
  set_idt_entry((interrupt_descriptors + 10), (uint32)&intr10);
  set_idt_entry((interrupt_descriptors + 11), (uint32)&intr11);
  set_idt_entry((interrupt_descriptors + 12), (uint32)&intr12);
  set_idt_entry((interrupt_descriptors + 13), (uint32)&intr13);
  set_idt_entry((interrupt_descriptors + 14), (uint32)&intr14);
  set_idt_entry((interrupt_descriptors + 15), (uint32)&intr15);
  set_idt_entry((interrupt_descriptors + 16), (uint32)&intr16);
  set_idt_entry((interrupt_descriptors + 17), (uint32)&intr17);
  set_idt_entry((interrupt_descriptors + 18), (uint32)&intr18);
  set_idt_entry((interrupt_descriptors + 19), (uint32)&intr19);
  set_idt_entry((interrupt_descriptors + 20), (uint32)&intr20);
  set_idt_entry((interrupt_descriptors + 21), (uint32)&intr21);
  set_idt_entry((interrupt_descriptors + 22), (uint32)&intr22);
  set_idt_entry((interrupt_descriptors + 23), (uint32)&intr23);
  set_idt_entry((interrupt_descriptors + 24), (uint32)&intr24);
  set_idt_entry((interrupt_descriptors + 25), (uint32)&intr25);
  set_idt_entry((interrupt_descriptors + 26), (uint32)&intr26);
  set_idt_entry((interrupt_descriptors + 27), (uint32)&intr27);
  set_idt_entry((interrupt_descriptors + 28), (uint32)&intr28);
  set_idt_entry((interrupt_descriptors + 29), (uint32)&intr29);
  set_idt_entry((interrupt_descriptors + 30), (uint32)&intr30);
  set_idt_entry((interrupt_descriptors + 31), (uint32)&intr31);
  set_idt_entry((interrupt_descriptors + 32), (uint32)&intr32);
  set_idt_entry((interrupt_descriptors + 33), (uint32)&intr33);
  set_idt_entry((interrupt_descriptors + 34), (uint32)&intr34);
  set_idt_entry((interrupt_descriptors + 35), (uint32)&intr35);
  set_idt_entry((interrupt_descriptors + 36), (uint32)&intr36);
  set_idt_entry((interrupt_descriptors + 37), (uint32)&intr37);
  set_idt_entry((interrupt_descriptors + 38), (uint32)&intr38);
  set_idt_entry((interrupt_descriptors + 39), (uint32)&intr39);
  set_idt_entry((interrupt_descriptors + 40), (uint32)&intr40);
  set_idt_entry((interrupt_descriptors + 41), (uint32)&intr41);
  set_idt_entry((interrupt_descriptors + 42), (uint32)&intr42);
  set_idt_entry((interrupt_descriptors + 43), (uint32)&intr43);
  set_idt_entry((interrupt_descriptors + 44), (uint32)&intr44);
  set_idt_entry((interrupt_descriptors + 45), (uint32)&intr45);
  set_idt_entry((interrupt_descriptors + 46), (uint32)&intr46);
  set_idt_entry((interrupt_descriptors + 47), (uint32)&intr47);
  set_idt_entry((interrupt_descriptors + 0x80), (uint32)&intx80);
  load_idt(&idt_pointer);

}
