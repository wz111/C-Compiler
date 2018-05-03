.data
max_num:	.word	1024
$string0:	.asciiz	"OVERFLOW! "
$string1:	.asciiz	"complete number: "
$string2:	.asciiz	"  "
$string3:	.asciiz	"  "
$string4:	.asciiz	"---------------------------------------------------------------\n"
$string5:	.asciiz	" "
$string6:	.asciiz	"  "
$string7:	.asciiz	"The total is "
.text
.globl main
		j	main
complete_num:
		sw	$ra	($sp)
		subi	$sp	$sp	4
		subi	$sp	$sp	4096
		subi	$sp	$sp	40
		li	$v0	2
		move	$s7	$v0
		sw	$v0	-4108($fp)
LABEL_0000:
		la	$v1	max_num
		lw	$v1	($v1)
		bge	$v0	$v1	LABEL_0001
		li	$v0	0
		li	$v1	1
		sub	$v0	$v0	$v1
		subi	$sp	$sp	4
		sw	$v0	-4144($fp)
		move	$s5	$v0
		sw	$v0	-4112($fp)
		move	$v0	$s7
		move	$s4	$v0
		sw	$v0	-4116($fp)
		li	$v0	1
		move	$s6	$v0
		sw	$v0	-4104($fp)
LABEL_0002:
		bge	$v0	$s7	LABEL_0003
		div	$v0	$s7	$s6
		subi	$sp	$sp	4
		sw	$v0	-4148($fp)
		mul	$v0	$v0	$s6
		subi	$sp	$sp	4
		sw	$v0	-4152($fp)
		sw	$v0	-4120($fp)
		bne	$v0	$s7	LABEL_0004
		li	$v1	1
		add	$v0	$s5	$v1
		subi	$sp	$sp	4
		sw	$v0	-4156($fp)
		move	$s5	$v0
		sw	$v0	-4112($fp)
		sub	$v0	$s4	$s6
		subi	$sp	$sp	4
		sw	$v0	-4160($fp)
		move	$s4	$v0
		sw	$v0	-4116($fp)
		li	$v1	1024
		blt	$s5	$v1	LABEL_0006
		li	$v0	4
		la	$a0	$string0
		syscall
		j	LABEL_0007
LABEL_0006:
		lw	$v0	-4104($fp)
		lw	$a0	-4112($fp)
		la	$a1	-8($fp)
		mul	$a0	$a0	-4
		add	$a0	$a0	$a1
		sw	$v0	($a0)
LABEL_0007:
		j	LABEL_0005
LABEL_0004:
LABEL_0005:
		li	$v1	1
		add	$v0	$s6	$v1
		subi	$sp	$sp	4
		sw	$v0	-4164($fp)
		move	$s6	$v0
		sw	$v0	-4104($fp)
		j	LABEL_0002
LABEL_0003:
		li	$v1	0
		bne	$s4	$v1	LABEL_0008
		li	$v0	4
		la	$a0	$string1
		syscall
		move	$a0	$s7
		li	$v0	1
		syscall
		li	$v0	0
		move	$s6	$v0
		sw	$v0	-4104($fp)
LABEL_0010:
		bgt	$v0	$s5	LABEL_0011
		lw	$a0	-4104($fp)
		la	$a1	-8($fp)
		mul	$a0	$a0	-4
		add	$a0	$a0	$a1
		lw	$v0	($a0)
		subi	$sp	$sp	4
		sw	$v0	-4168($fp)
		li	$v0	4
		la	$a0	$string2
		syscall
		lw	$a0	-4168($fp)
		li	$v0	1
		syscall
		li	$v1	1
		add	$v0	$s6	$v1
		subi	$sp	$sp	4
		sw	$v0	-4172($fp)
		move	$s6	$v0
		sw	$v0	-4104($fp)
		j	LABEL_0010
LABEL_0011:
		li	$v0	4
		la	$a0	$string3
		syscall
		j	LABEL_0009
LABEL_0008:
LABEL_0009:
		li	$v1	1
		add	$v0	$s7	$v1
		subi	$sp	$sp	4
		sw	$v0	-4176($fp)
		move	$s7	$v0
		sw	$v0	-4108($fp)
		j	LABEL_0000
LABEL_0001:
		li	$v0	4
		la	$a0	$string4
		syscall
		li	$v0	0
		move	$s3	$v0
		sw	$v0	-4132($fp)
		li	$v0	1
		move	$s2	$v0
		sw	$v0	-4136($fp)
		li	$v0	2
		move	$s1	$v0
		sw	$v0	-4124($fp)
LABEL_0012:
		la	$v1	max_num
		lw	$v1	($v1)
		bgt	$v0	$v1	LABEL_0013
		li	$v1	1
		sub	$v0	$s1	$v1
		subi	$sp	$sp	4
		sw	$v0	-4180($fp)
		move	$s0	$v0
		sw	$v0	-4128($fp)
		li	$v0	2
		move	$s6	$v0
		sw	$v0	-4104($fp)
LABEL_0014:
		bgt	$v0	$s0	LABEL_0015
		div	$v0	$s1	$s6
		subi	$sp	$sp	4
		sw	$v0	-4184($fp)
		mul	$v0	$v0	$s6
		subi	$sp	$sp	4
		sw	$v0	-4188($fp)
		sw	$v0	-4140($fp)
		bne	$v0	$s1	LABEL_0016
		li	$v0	0
		move	$s2	$v0
		sw	$v0	-4136($fp)
		j	LABEL_0017
LABEL_0016:
LABEL_0017:
		li	$v1	1
		add	$v0	$s6	$v1
		subi	$sp	$sp	4
		sw	$v0	-4192($fp)
		move	$s6	$v0
		sw	$v0	-4104($fp)
		j	LABEL_0014
LABEL_0015:
		li	$v1	1
		bne	$s2	$v1	LABEL_0018
		li	$v0	4
		la	$a0	$string5
		syscall
		move	$a0	$s1
		li	$v0	1
		syscall
		li	$v1	1
		add	$v0	$s3	$v1
		subi	$sp	$sp	4
		sw	$v0	-4196($fp)
		move	$s3	$v0
		sw	$v0	-4132($fp)
		li	$v1	10
		div	$v0	$s3	$v1
		subi	$sp	$sp	4
		sw	$v0	-4200($fp)
		li	$v1	10
		mul	$v0	$v0	$v1
		subi	$sp	$sp	4
		sw	$v0	-4204($fp)
		sw	$v0	-4140($fp)
		bne	$v0	$s3	LABEL_0020
		li	$v0	4
		la	$a0	$string6
		syscall
		j	LABEL_0021
LABEL_0020:
LABEL_0021:
		j	LABEL_0019
LABEL_0018:
LABEL_0019:
		li	$v0	1
		move	$s2	$v0
		sw	$v0	-4136($fp)
		li	$v1	1
		add	$v0	$s1	$v1
		subi	$sp	$sp	4
		sw	$v0	-4208($fp)
		move	$s1	$v0
		sw	$v0	-4124($fp)
		j	LABEL_0012
LABEL_0013:
		li	$v0	4
		la	$a0	$string7
		syscall
		move	$a0	$s3
		li	$v0	1
		syscall
		lw	$ra	-4($fp)
		move	$t0	$ra
		move	$sp	$fp
		lw	$fp	($fp)
		jr	$t0
main:
		move	$fp	$sp
		sw	$fp	($sp)
		subi	$sp	$sp	4
		sw	$ra	($sp)
		subi	$sp	$sp	4
		subi	$sp	$sp	8
		sw	$fp	4($sp)
		addi	$fp	$sp	4
		jal	complete_num
		li	$v0	10
		syscall
