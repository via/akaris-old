CFLAGS = -std=c89 -O0 -g -Wall -Wextra -Werror  -nostdlib -nostartfiles -nodefaultlibs -I../include/ -c -fno-builtin
LDFLAGS = -T linker.ld
CC=gcc
OBJS = test2.o test1.o

default: ${OBJS}
	ld ${LDFLAGS} -o test2.k test2.o
	ld ${LDFLAGS} -o test1.k test1.o


install:
#	mdconfig -a -t vnode -f ../bootdisk.img
	mount -o loop ../bootdisk.img ../../mnt
	cp test2.k test1.k ../../mnt/
	umount ../../mnt
	#mdconfig -d -u 0
clean:
	-rm *.k
	-rm *.o
	-rm *.lst
.c.o:
	${CC} -o $@ -Wa,-a,-ad ${CFLAGS} $< > ${<}.lst

.s.o:
	${CC} -o $@ ${CFLAGS} $<

