/* start.s -- Multiboot compatible loader */

.global start                 # making entry point visible to linker

# setting up the Multiboot header - see GRUB docs for details
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum required

.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# reserve initial kernel stack space
.globl stack
.set STACKSIZE, 0x4000          # that is, 16k.
.comm stack, STACKSIZE, 32      # reserve 16k stack on a quadword boundary

start:
   mov   $(stack + STACKSIZE), %esp # set up the stack
   push  %eax                       # Multiboot magic number
   push  %ebx                       # Multiboot data structure

   call  cmain            # call kernel proper

   cli
hang:
   sti

	ljmp $0x08, $0x40000000
	jmp   hang


