CFLAGS = -std=c89 -O0 -g -Wall -Wextra -Werror  -nostdlib -nostartfiles -nodefaultlibs -I../include/ -c -fno-builtin
LDFLAGS = -T linker.ld

#OBJS = start.o cmain.o cpuid.o bootvideo.o asmutil.o gdt.o asm_interrupts.o interrupt.o pic.o physical_memory.o slab.o paging.o context.o
OBJS = test2.o
default: ${OBJS}
	ld ${LDFLAGS} -o test2.k test2.o
install:
#	mdconfig -a -t vnode -f ../bootdisk.img
	mount -o loop ../bootdisk.img /mnt/floppy
	cp test2.k /mnt/floppy
	umount /mnt/floppy
	#mdconfig -d -u 0
clean:
	-rm *.k
	-rm *.o
	-rm *.lst
.c.o:
	gcc -o $@ -Wa,-a,-ad ${CFLAGS} $< > ${<}.lst

.s.o:
	gcc -o $@ ${CFLAGS} $<

