#ifndef I386_SYSCALL_H
#define I386_SYSCALL_H

#include <i386/interrupt.h>
#include <i386/process.h>
#include <i386/context.h>

#define SYSCALL_MAX_SYSCALLS 9

#define SYSCALL_FUNCTION_KDEBUG 1
#define SYSCALL_FUNCTION_MMAP 2
#define SYSCALL_FUNCTION_MAILBOX_CREATE 3
#define SYSCALL_FUNCTION_MAILBOX_SEND 4
#define SYSCALL_FUNCTION_MAILBOX_RECEIVE 5
#define SYSCALL_FUNCTION_MMAP_LIST 6
#define SYSCALL_FUNCTION_BLOCK 7

#define SYSCALL_LINK_IRQ 8
#define SYSCALL_IO 9


/*For ALL syscalls, EAX defines the function, EDX defines parameter*/

  /*Mailbox Send: EAX = 4 EDX = Message ECX = Dest PID*/
/*Mailbox Create: EAX = 3 EDX = Max Messages ECX = PID filter*/



void enable_syscall();
void syscall_handler(isr_regs *);


#endif
