CFLAGS = -std=c89 -O0 -g -Wall -Wextra -Werror  -nostdlib -nostartfiles -nodefaultlibs -I../include/ -c -fno-builtin
LDFLAGS = -T linker.ld
CC=i686-elf-gcc
LD=i686-elf-ld
#OBJS = test2.o test1.o kbd.o
OBJS = test3.o  kbd.o



default: libak/libak.o ${OBJS}
	${LD} ${LDFLAGS} -o test3.k test3.o libak/libak.o
	${LD} ${LDFLAGS} -o kbd.k kbd.o libak/libak.o
libak/libak.o:
	(cd libak; make);

install:
#	mdconfig -a -t vnode -f ../bootdisk.img
	mount -o loop ../bootdisk.img ../../mnt
	cp test3.k ../../mnt/
	cp kbd.k ../../mnt/
	umount ../../mnt
	#mdconfig -d -u 0
clean:
	-rm *.k
	-rm *.o
	-rm *.lst
	(cd libak; make clean;);
.c.o:
	${CC} -o $@ -Wa,-a,-ad ${CFLAGS} $< > ${<}.lst

.s.o:
	${CC} -o $@ ${CFLAGS} $<

