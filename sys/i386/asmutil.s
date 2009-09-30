.globl load_gdt
load_gdt:
	movl 4(%esp), %eax
	lgdt (%eax)

	ljmp $0x08, $reload_cs
reload_cs:
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss
	ret

.globl load_tr
load_tr:
	movw 4(%esp), %ax
	ltr %ax
	ret

  .globl test_and_set
test_and_set:
	movl 4(%esp),%eax #   get new_value into %eax
	movl 8(%esp),%edx #   get lock_pointer into %edx
	lock                # next instruction is locked
	xchgl %eax,(%edx) #   swap %eax with what is stored in (%edx)
        	# ... and don't let any other cpu touch that
	ret
	