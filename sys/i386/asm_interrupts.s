
.globl intr00
.globl intr01
.globl intr02
.globl intr03
.globl intr04
.globl intr05
.globl intr06
.globl intr07
.globl intr08
.globl intr09
.globl intr10
.globl intr11
.globl intr12
.globl intr13
.globl intr14
.globl intr15
.globl intr16
.globl intr17
.globl intr18
.globl intr19
.globl intr20
.globl intr21
.globl intr22
.globl intr23
.globl intr24
.globl intr25
.globl intr26
.globl intr27
.globl intr28
.globl intr29
.globl intr30
.globl intr31
.globl intr32
.globl intr33
.globl intr34
.globl intr35
.globl intr36
.globl intr37
.globl intr38
.globl intr39
.globl intr40
.globl intr41
.globl intr42
.globl intr43
.globl intr44
.globl intr45
.globl intr46
.globl intr47
.globl intx80

intx80:
	pushl $0x0
	pushl $0x80
	jmp int_hand
intr00:
	pushl $0
	pushl $0
	jmp int_hand
intr01:
	pushl $0
	pushl $1
	jmp int_hand
intr02:
	pushl $0
	pushl $2
	jmp int_hand
intr03:
	pushl $0
	pushl $3
	jmp int_hand
intr04:
	pushl $0
	pushl $4
	jmp int_hand
intr05:
	pushl $0
	pushl $5
	jmp int_hand
intr06:
	pushl $0
	pushl $6
	jmp int_hand
intr07:
	pushl $0
	pushl $7
	jmp int_hand
intr08:
	pushl $8
	jmp int_hand
intr09:
	pushl $0
	pushl $9
	jmp int_hand
intr10:
	pushl $10
	jmp int_hand
intr11:
	pushl $11
	jmp int_hand
intr12:
	pushl $12
	jmp int_hand
intr13:
	pushl $13
	jmp int_hand
intr14:
	pushl $14
	jmp int_hand
intr15:
	pushl $0
	pushl $15
	jmp int_hand
intr16:
	pushl $0
	pushl $16
	jmp int_hand
intr17:
	pushl $17
	jmp int_hand
intr18:
	pushl $0
	pushl $18
	jmp int_hand
intr19:
	pushl $0
	pushl $19
	jmp int_hand
intr20:
	pushl $0
	pushl $20
	jmp int_hand
intr21:
	pushl $0
	pushl $21
	jmp int_hand
intr22:
	pushl $0
	pushl $22
	jmp int_hand
intr23:
	pushl $0
	pushl $23
	jmp int_hand
intr24:
	pushl $0
	pushl $24
	jmp int_hand
intr25:
	pushl $0
	pushl $25
	jmp int_hand
intr26:
	pushl $0
	pushl $26
	jmp int_hand
intr27:
	pushl $0
	pushl $27
	jmp int_hand
intr28:
	pushl $0
	pushl $28
	jmp int_hand
intr29:
	pushl $0
	pushl $29
	jmp int_hand
intr30:
	pushl $0
	pushl $30
	jmp int_hand
intr31:
	pushl $0
	pushl $31
	jmp int_hand
intr32:
	pushl $0
	pushl $32
	jmp int_hand
intr33:
	pushl $0
	pushl $33
	jmp int_hand
intr34:
	pushl $0
	pushl $34
	jmp int_hand

intr35:
	pushl $0
	pushl $35
	jmp int_hand
intr36:
	pushl $0
	pushl $36
	jmp int_hand
intr37:
	pushl $0
	pushl $37
	jmp int_hand
intr38:
	pushl $0
	pushl $38
	jmp int_hand
intr39:
	pushl $0
	pushl $39
	jmp int_hand
intr40:
	pushl $0
	pushl $40
	jmp int_hand
intr41:
	pushl $0
	pushl $41
	jmp int_hand
intr42:
	pushl $0
	pushl $42
	jmp int_hand
intr43:
	pushl $0
	pushl $43
	jmp int_hand
intr44:
	pushl $0
	pushl $44
	jmp int_hand
intr45:
	pushl $0
	pushl $45
	jmp int_hand
intr46:
	pushl $0
	pushl $46
	jmp int_hand
intr47:
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
	
