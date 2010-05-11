CFLAGS = -std=c89 -O0 -g -Wall -Wextra -Werror  -nostdlib -nostartfiles -nodefaultlibs -I../../include/ -c -fno-builtin
CC=gcc
OBJS = syscall.o

default: ${OBJS}
	@ld ${LDFLAGS} -o libak.o ${OBJS} -r
	@echo "[LD] libak.o"

clean:
	-rm *.o
	-rm *.lst
.c.o:
	@ ${CC} -o $@ -Wa,-a,-ad ${CFLAGS} $< > ${<}.lst
	@echo "[CC] ${<}"
.s.o:
	@ ${CC} -o $@ -Wa,-a,-ad  ${CFLAGS} $< > ${<}.lst
	@echo "[AS] ${<}"
