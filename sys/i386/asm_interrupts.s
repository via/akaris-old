
.globl int00
.globl int01
.globl int02
.globl int03
.globl int04
.globl int05
.globl int06
.globl int07
.globl int08
.globl int09
.globl int10
.globl int11
.globl int12
.globl int13
.globl int14
.globl int15
.globl int16
.globl int17
.globl int18
.globl int19
.globl int20
.globl int21
.globl int22
.globl int23
.globl int24
.globl int25
.globl int26
.globl int27
.globl int28
.globl int29
.globl int30
.globl int31
.globl int32
.globl int33
.globl int34
.globl int35
.globl int36
.globl int37
.globl int38
.globl int39
.globl int40
.globl int41
.globl int42
.globl int43
.globl int44
.globl int45
.globl int46
.globl int47
.globl intx80

intx80:
	pushl $0x0
	pushl $0x80
	jmp int_hand
int00:
	pushl $0
	pushl $0
	jmp int_hand
int01:
	pushl $0
	pushl $1
	jmp int_hand
int02:
	pushl $0
	pushl $2
	jmp int_hand
int03:
	pushl $0
	pushl $3
	jmp int_hand
int04:
	pushl $0
	pushl $4
	jmp int_hand
int05:
	pushl $0
	pushl $5
	jmp int_hand
int06:
	pushl $0
	pushl $6
	jmp int_hand
int07:
	pushl $0
	pushl $7
	jmp int_hand
int08:
	pushl $8
	jmp int_hand
int09:
	pushl $0
	pushl $9
	jmp int_hand
int10:
	pushl $10
	jmp int_hand
int11:
	pushl $11
	jmp int_hand
int12:
	pushl $12
	jmp int_hand
int13:
	pushl $13
	jmp int_hand
int14:
	pushl $14
	jmp int_hand
int15:
	pushl $0
	pushl $15
	jmp int_hand
int16:
	pushl $0
	pushl $16
	jmp int_hand
int17:
	pushl $17
	jmp int_hand
int18:
	pushl $0
	pushl $18
	jmp int_hand
int19:
	pushl $0
	pushl $19
	jmp int_hand
int20:
	pushl $0
	pushl $20
	jmp int_hand
int21:
	pushl $0
	pushl $21
	jmp int_hand
int22:
	pushl $0
	pushl $22
	jmp int_hand
int23:
	pushl $0
	pushl $23
	jmp int_hand
int24:
	pushl $0
	pushl $24
	jmp int_hand
int25:
	pushl $0
	pushl $25
	jmp int_hand
int26:
	pushl $0
	pushl $26
	jmp int_hand
int27:
	pushl $0
	pushl $27
	jmp int_hand
int28:
	pushl $0
	pushl $28
	jmp int_hand
int29:
	pushl $0
	pushl $29
	jmp int_hand
int30:
	pushl $0
	pushl $30
	jmp int_hand
int31:
	pushl $0
	pushl $31
	jmp int_hand
int32:
	pushl $0
	pushl $32
	jmp int_hand
int33:
	pushl $0
	pushl $33
	jmp int_hand
int34:
	pushl $0
	pushl $34
	jmp int_hand

int35:
	pushl $0
	pushl $35
	jmp int_hand
int36:
	pushl $0
	pushl $36
	jmp int_hand
int37:
	pushl $0
	pushl $37
	jmp int_hand
int38:
	pushl $0
	pushl $38
	jmp int_hand
int39:
	pushl $0
	pushl $39
	jmp int_hand
int40:
	pushl $0
	pushl $40
	jmp int_hand
int41:
	pushl $0
	pushl $41
	jmp int_hand
int42:
	pushl $0
	pushl $42
	jmp int_hand
int43:
	pushl $0
	pushl $43
	jmp int_hand
int44:
	pushl $0
	pushl $44
	jmp int_hand
int45:
	pushl $0
	pushl $45
	jmp int_hand
int46:
	pushl $0
	pushl $46
	jmp int_hand
int47:
	pushl $0
	pushl $47
	jmp int_hand
.globl begin_schedule
.globl load_idt
load_idt:
	mov 4(%esp), %eax
	lidt (%eax)
	ret

int_hand:	
	pusha
	pushl %ds
	pushl %es
	pushl %fs
	pushl %gs
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movl %esp, %eax
	pushl %eax
	call c_isr
schedule_inject:	
	mov %eax, %esp
	popl %gs
	popl %fs
	popl %es
	popl %ds
	popa
	add $8, %esp
	
	
	iret

begin_schedule:
	movl 4(%esp), %eax
	jmp schedule_inject
	