	.text
	.file	"input.ju"
	.globl	PdTQ1CTPy4FTxKfrtc4LE_main      # -- Begin function PdTQ1CTPy4FTxKfrtc4LE_main
	.p2align	2
	.type	PdTQ1CTPy4FTxKfrtc4LE_main,@function
	.section	.opd,"aw",@progbits
PdTQ1CTPy4FTxKfrtc4LE_main:             # @PdTQ1CTPy4FTxKfrtc4LE_main
	.p2align	3
	.quad	.Lfunc_begin0
	.quad	.TOC.@tocbase
	.quad	0
	.text
.Lfunc_begin0:
	.cfi_startproc
# %bb.0:                                # %entry
	mflr 0
	std 0, 16(1)
	stdu 1, -128(1)
	.cfi_def_cfa_offset 128
	.cfi_offset lr, 16
	.cfi_offset r30, -16
	addis 3, 2, .LC0@toc@ha
	addis 4, 2, .LC1@toc@ha
	std 30, 112(1)                          # 8-byte Folded Spill
	addis 5, 2, .LC2@toc@ha
	ld 3, .LC0@toc@l(3)
	ld 30, .LC1@toc@l(4)
	li 4, 12
	stw 4, 0(3)
	ld 3, .LC2@toc@l(5)
	li 4, 9
	stw 4, 0(30)
	li 4, 162
	stw 4, 0(3)
	li 3, 162
	li 4, 18
	bl YbrHmjNBLeBvMXGvhC_Ie_sum_XV.dJjVe4b4H.dnFVU4RT0
	nop
	addis 4, 2, .LC3@toc@ha
	lwz 5, 0(30)
	ld 4, .LC3@toc@l(4)
	stw 3, 0(4)
	rlwinm 3, 5, 1, 0, 30
	bl YbrHmjNBLeBvMXGvhC_Ie_sum_A3fwUMnOhY33O0Qcu4z8IM
	nop
	addis 4, 2, .LC4@toc@ha
	ld 30, 112(1)                           # 8-byte Folded Reload
	ld 5, .LC4@toc@l(4)
	mr	4, 3
	li 3, 0
	stw 4, 0(5)
	addi 1, 1, 128
	ld 0, 16(1)
	mtlr 0
	blr
	.long	0
	.quad	0
.Lfunc_end0:
	.size	PdTQ1CTPy4FTxKfrtc4LE_main, .Lfunc_end0-.Lfunc_begin0
	.cfi_endproc
                                        # -- End function
	.globl	main                            # -- Begin function main
	.p2align	2
	.type	main,@function
	.section	.opd,"aw",@progbits
main:                                   # @main
	.p2align	3
	.quad	.Lfunc_begin1
	.quad	.TOC.@tocbase
	.quad	0
	.text
.Lfunc_begin1:
	.cfi_startproc
# %bb.0:                                # %entry
	mflr 0
	std 0, 16(1)
	stdu 1, -112(1)
	.cfi_def_cfa_offset 112
	.cfi_offset lr, 16
	bl PdTQ1CTPy4FTxKfrtc4LE_main
	nop
	addi 1, 1, 112
	ld 0, 16(1)
	mtlr 0
	blr
	.long	0
	.quad	0
.Lfunc_end1:
	.size	main, .Lfunc_end1-.Lfunc_begin1
	.cfi_endproc
                                        # -- End function
	.type	a,@object                       # @a
	.section	.bss,"aw",@nobits
	.globl	a
	.p2align	2
a:
	.long	0                               # 0x0
	.size	a, 4

	.type	b,@object                       # @b
	.globl	b
	.p2align	2
b:
	.long	0                               # 0x0
	.size	b, 4

	.type	c,@object                       # @c
	.globl	c
	.p2align	2
c:
	.long	0                               # 0x0
	.size	c, 4

	.type	d,@object                       # @d
	.globl	d
	.p2align	2
d:
	.long	0                               # 0x0
	.size	d, 4

	.type	e,@object                       # @e
	.globl	e
	.p2align	2
e:
	.long	0                               # 0x0
	.size	e, 4

	.section	".note.GNU-stack","",@progbits
	.section	.toc,"aw",@progbits
.LC0:
	.tc b[TC],b
.LC1:
	.tc a[TC],a
.LC2:
	.tc c[TC],c
.LC3:
	.tc d[TC],d
.LC4:
	.tc e[TC],e
