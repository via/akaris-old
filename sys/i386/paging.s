	.file	"paging.c"
	.text
	.p2align 4,,15
.globl get_kernel_page_tables
	.type	get_kernel_page_tables, @function
get_kernel_page_tables:
	pushl	%ebp
	movl	%esp, %ebp
	movl	$kernel_page_tables, %eax
	popl	%ebp
	ret
	.size	get_kernel_page_tables, .-get_kernel_page_tables
	.p2align 4,,15
.globl initialize_kernel_paging
	.type	initialize_kernel_paging, @function
initialize_kernel_paging:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$36, %esp
	movl	$0, (%esp)
	call	allocate_page
	sall	$12, %eax
	movl	%eax, kernel_pd
	movl	kernel_pd, %eax
	movl	$4096, 8(%esp)
	movl	$0, 4(%esp)
	movl	%eax, (%esp)
	call	memset
	movl	$0, -8(%ebp)
	jmp	.L4
.L5:
	movl	-8(%ebp), %ebx
	movl	$0, (%esp)
	call	allocate_page
	sall	$12, %eax
	movl	%eax, kernel_page_tables(,%ebx,4)
	movl	-8(%ebp), %eax
	movl	kernel_page_tables(,%eax,4), %eax
	movl	$4096, 8(%esp)
	movl	$0, 4(%esp)
	movl	%eax, (%esp)
	call	memset
	movl	-8(%ebp), %eax
	movl	kernel_page_tables(,%eax,4), %eax
	movl	%eax, %ecx
	movl	-8(%ebp), %eax
	sall	$2, %eax
	movl	%eax, %edx
	movl	kernel_pd, %eax
	leal	(%edx,%eax), %eax
	movl	$259, 8(%esp)
	movl	%ecx, 4(%esp)
	movl	%eax, (%esp)
	call	set_pde
	addl	$1, -8(%ebp)
.L4:
	cmpl	$15, -8(%ebp)
	jle	.L5
	movl	$0, -8(%ebp)
	jmp	.L7
.L8:
	movl	-8(%ebp), %eax
	movl	%eax, %ecx
	sall	$12, %ecx
	movl	kernel_page_tables, %edx
	movl	-8(%ebp), %eax
	sall	$2, %eax
	leal	(%edx,%eax), %eax
	movl	$259, 8(%esp)
	movl	%ecx, 4(%esp)
	movl	%eax, (%esp)
	call	set_pte
	addl	$1, -8(%ebp)
.L7:
	cmpl	$1023, -8(%ebp)
	jle	.L8
	movl	$page_fault_handler, 4(%esp)
	movl	$14, (%esp)
	call	link_irq
	movl	kernel_pd, %eax
	movl	%eax, (%esp)
	call	set_cr3
	call	enable_paging
	addl	$36, %esp
	popl	%ebx
	popl	%ebp
	ret
	.size	initialize_kernel_paging, .-initialize_kernel_paging
	.p2align 4,,15
.globl set_pte
	.type	set_pte, @function
set_pte:
	pushl	%ebp
	movl	%esp, %ebp
	movl	12(%ebp), %eax
	movl	%eax, %edx
	andl	$-4096, %edx
	movl	16(%ebp), %eax
	leal	(%edx,%eax), %eax
	movl	%eax, %edx
	movl	8(%ebp), %eax
	movl	%edx, (%eax)
	popl	%ebp
	ret
	.size	set_pte, .-set_pte
	.p2align 4,,15
.globl set_pde
	.type	set_pde, @function
set_pde:
	pushl	%ebp
	movl	%esp, %ebp
	movl	12(%ebp), %eax
	movl	%eax, %edx
	andl	$-4096, %edx
	movl	16(%ebp), %eax
	leal	(%edx,%eax), %eax
	movl	%eax, %edx
	movl	8(%ebp), %eax
	movl	%edx, (%eax)
	popl	%ebp
	ret
	.size	set_pde, .-set_pde
	.p2align 4,,15
.globl set_cr3
	.type	set_cr3, @function
set_cr3:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	andl	$4095, %eax
	testl	%eax, %eax
	jne	.L18
	movl	8(%ebp), %eax
#APP
	movl %eax, %cr3
#NO_APP
.L18:
	popl	%ebp
	ret
	.size	set_cr3, .-set_cr3
	.p2align 4,,15
.globl get_cr3
	.type	get_cr3, @function
get_cr3:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
#APP
	movl %cr3, %eax
#NO_APP
	movl	%eax, -4(%ebp)
	movl	-4(%ebp), %eax
	leave
	ret
	.size	get_cr3, .-get_cr3
	.p2align 4,,15
.globl enable_paging
	.type	enable_paging, @function
enable_paging:
	pushl	%ebp
	movl	%esp, %ebp
#APP
	movl %cr0, %eax
orl $0x80000000, %eax
movl %eax, %cr0
#NO_APP
	popl	%ebp
	ret
	.size	enable_paging, .-enable_paging
	.section	.rodata
.LC0:
	.string	"Page Fault! %x \n"
	.text
	.p2align 4,,15
.globl page_fault_handler
	.type	page_fault_handler, @function
page_fault_handler:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	8(%ebp), %eax
	movl	48(%eax), %eax
	movl	%eax, 4(%esp)
	movl	$.LC0, (%esp)
	call	bootvideo_printf
.L24:
	jmp	.L24
	.size	page_fault_handler, .-page_fault_handler
	.p2align 4,,15
.globl get_unused_kernel_virtual_page
	.type	get_unused_kernel_virtual_page, @function
get_unused_kernel_virtual_page:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	movl	$0, -4(%ebp)
	movl	$1, -12(%ebp)
	jmp	.L27
.L28:
	movl	$0, -8(%ebp)
	jmp	.L29
.L30:
	movl	-12(%ebp), %eax
	movl	kernel_page_tables(,%eax,4), %edx
	movl	-8(%ebp), %eax
	sall	$2, %eax
	leal	(%edx,%eax), %eax
	movl	(%eax), %eax
	andl	$512, %eax
	testl	%eax, %eax
	jne	.L31
	movl	$1, -4(%ebp)
	jmp	.L33
.L31:
	addl	$1, -8(%ebp)
.L29:
	cmpl	$1023, -8(%ebp)
	jle	.L30
.L33:
	cmpl	$1, -4(%ebp)
	je	.L34
	addl	$1, -12(%ebp)
.L27:
	cmpl	$15, -12(%ebp)
	jle	.L28
.L34:
	movl	-12(%ebp), %eax
	movl	kernel_page_tables(,%eax,4), %edx
	movl	-8(%ebp), %eax
	sall	$2, %eax
	leal	(%edx,%eax), %ecx
	movl	-12(%ebp), %eax
	movl	kernel_page_tables(,%eax,4), %edx
	movl	-8(%ebp), %eax
	sall	$2, %eax
	leal	(%edx,%eax), %eax
	movl	(%eax), %eax
	orl	$771, %eax
	movl	%eax, (%ecx)
	movl	-12(%ebp), %eax
	sall	$10, %eax
	addl	-8(%ebp), %eax
	sall	$12, %eax
	leave
	ret
	.size	get_unused_kernel_virtual_page, .-get_unused_kernel_virtual_page
	.section	.rodata
.LC1:
	.string	"Out_of_memory!\n"
	.text
	.p2align 4,,15
.globl get_usable_kernel_virtual_page
	.type	get_usable_kernel_virtual_page, @function
get_usable_kernel_virtual_page:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$24, %esp
	call	get_unused_kernel_virtual_page
	movl	%eax, -16(%ebp)
	movl	-16(%ebp), %edx
	movl	%edx, %eax
	sarl	$31, %eax
	shrl	$20, %eax
	addl	%edx, %eax
	sarl	$12, %eax
	movl	%eax, -16(%ebp)
	movl	-16(%ebp), %edx
	movl	%edx, %eax
	sarl	$31, %eax
	shrl	$22, %eax
	addl	%edx, %eax
	sarl	$10, %eax
	movl	%eax, -12(%ebp)
	movl	-16(%ebp), %edx
	movl	%edx, %eax
	sarl	$31, %eax
	movl	%eax, %ecx
	shrl	$22, %ecx
	leal	(%edx,%ecx), %eax
	andl	$1023, %eax
	subl	%ecx, %eax
	movl	%eax, -8(%ebp)
	movl	$0, (%esp)
	call	allocate_page
	sall	$12, %eax
	movl	%eax, -4(%ebp)
	cmpl	$0, -4(%ebp)
	jne	.L38
	movl	$.LC1, (%esp)
	call	bootvideo_printf
.L40:
	jmp	.L40
.L38:
	movl	-12(%ebp), %eax
	movl	kernel_page_tables(,%eax,4), %edx
	movl	-8(%ebp), %eax
	sall	$2, %eax
	leal	(%edx,%eax), %ecx
	movl	-12(%ebp), %eax
	movl	kernel_page_tables(,%eax,4), %edx
	movl	-8(%ebp), %eax
	sall	$2, %eax
	leal	(%edx,%eax), %eax
	movl	(%eax), %eax
	orl	-4(%ebp), %eax
	movl	%eax, (%ecx)
	movl	-16(%ebp), %eax
	sall	$12, %eax
#APP
	invlpg (%eax)
#NO_APP
	movl	-16(%ebp), %eax
	sall	$12, %eax
	leave
	ret
	.size	get_usable_kernel_virtual_page, .-get_usable_kernel_virtual_page
	.p2align 4,,15
.globl get_physical_address_from_kernel_virtual
	.type	get_physical_address_from_kernel_virtual, @function
get_physical_address_from_kernel_virtual:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$20, %esp
	cmpl	$67108863, 8(%ebp)
	jg	.L43
	movl	$-1, -20(%ebp)
	jmp	.L45
.L43:
	movl	8(%ebp), %edx
	movl	%edx, %eax
	sarl	$31, %eax
	shrl	$20, %eax
	addl	%edx, %eax
	sarl	$12, %eax
	movl	%eax, 8(%ebp)
	movl	8(%ebp), %edx
	movl	%edx, %eax
	sarl	$31, %eax
	shrl	$22, %eax
	addl	%edx, %eax
	sarl	$10, %eax
	movl	%eax, -12(%ebp)
	movl	8(%ebp), %edx
	movl	%edx, %eax
	sarl	$31, %eax
	movl	%eax, %ecx
	shrl	$22, %ecx
	leal	(%edx,%ecx), %eax
	andl	$1023, %eax
	subl	%ecx, %eax
	movl	%eax, -8(%ebp)
	movl	-12(%ebp), %eax
	movl	kernel_page_tables(,%eax,4), %edx
	movl	-8(%ebp), %eax
	sall	$2, %eax
	leal	(%edx,%eax), %eax
	movl	(%eax), %eax
	andl	$-4096, %eax
	movl	%eax, -4(%ebp)
	movl	-4(%ebp), %eax
	movl	%eax, -20(%ebp)
.L45:
	movl	-20(%ebp), %eax
	leave
	ret
	.size	get_physical_address_from_kernel_virtual, .-get_physical_address_from_kernel_virtual
	.comm	kernel_pd,4,4
	.comm	kernel_page_tables,64,32
	.ident	"GCC: (GNU) 4.2.1 20070719  [FreeBSD]"
