.data
.text
.globl main
		j	main
main:
		move	$fp	$sp
		sw	$fp	($sp)
		subi	$sp	$sp	4
		sw	$ra	($sp)
		subi	$sp	$sp	4
		subi	$sp	$sp	4
		li	$v0	0
		la	$a0	     
		la	$a1	-8($fp)
		mul	$a0	$a0	-4
		add	$a0	$a0	$a1
		sw	$v0	($a0)
		li	$v0	10
		syscall
