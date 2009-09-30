#ifndef I386_SYSCALL_H
#define I386_SYSCALL_H

#include <i386/interrupt.h>
#include <i386/process.h>
#include <i386/context.h>

#define SYSCALL_MAX_SYSCALLS 5

#define SYSCALL_FUNCTION_KDEBUG 1
#define SYSCALL_FUNCTION_MMAP 2
#define SYSCALL_FUNCTION_MAILBOX_CREATE 3
#define SYSCALL_FUNCTION_MAILBOX_SEND 4
#define SYSCALL_FUNCTION_MAILBOX_RECEIVE 5

/*For ALL syscalls, EAX defines the function, EDX defines parameter*/


void enable_syscall();
void syscall_handler(isr_regs *);


#endif