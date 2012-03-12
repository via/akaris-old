CFLAGS = -std=c89 -O0 -g -Wall -Wextra -Werror  -nostdlib -nostartfiles -nodefaultlibs -Iinclude/ -c -fno-builtin 
LDFLAGS = -T linker.ld
LD=i686-elf-ld
CC=i686-elf-gcc
OBJS = start.o cmain.o cpuid.o bootvideo.o asmutil.o gdt.o asm_interrupts.o interrupt.o pic.o physical_memory.o slab.o paging.o context.o process.o syscall.o  kfifo.o device_interface.o kqueue.o

COMMON_OBJS = common/slab.o common/process.o common/kfifo.o common/device_interface.o common/kqueue.o
I386_OBJS = i386/start.o i386/cmain.o i386/cpuid.o i386/bootvideo.o i386/asmutil.o i386/gdt.o i386/asm_interrupts.o i386/interrupt.o i386/pic.o i386/physical_memory.o i386/paging.o i386/context.o i386/syscall.o
X86_64_OBJS = x86_64/start.o x86_64/cmain.o

all: i386 servers

.PHONY : servers
servers:
	(cd servers; make)
i386: ${I386_OBJS} ${COMMON_OBJS}
	@${LD} -T i386/linker.ld -o i386/i386.k ${I386_OBJS} ${COMMON_OBJS}
	@echo " [LD] i386/i386.k"
install:
#	mdconfig -a -t vnode -f ../bootdisk.img
	mount -o loop bootdisk.img ../mnt
	-cp i386/i386.k ../mnt/
	-cp servers/*.k ../mnt/
	umount ../mnt
	#mdconfig -d -u 0
clean:
	-@rm */*.k 2>/dev/null
	-@rm */*.o 2>/dev/null
	-@rm */*.lst 2>/dev/null
	(cd servers; make clean)
	@echo " [CLEAN]"
.c.o:
	@ ${CC} -o $@ -Wa,-a,-ad ${CFLAGS} $< > ${<}.lst
	@echo " [CC] ${@}"
.s.o:
	@ ${CC} -o $@ -Wa,-a,-ad  ${CFLAGS} $< > ${<}.lst
	@echo " [AS] ${@}"
