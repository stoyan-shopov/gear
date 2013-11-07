.code 32

nop
add	r0,	r0,	#0x0
str	r0, [r0]
add	r0,	r15,	#0x0
add	r0,	r15,	#0x0
stm	r0,	{r0-r15}
stm	r0,	{r0-r14}

ldm	r0,	{ r0, r14 }
ldr	r0,	[r0]
add	r0,	#0
mov	r0,	#0
mrs	r0,	spsr
mrs	r0,	cpsr
msr	cpsr,	r0
msr	spsr,	r0
msr	cpsr_c,	r0

b	. - 4 * 4

msr	cpsr_c,	#(0xd0 | 0x3)	/* enter svc	*/
msr	cpsr_c,	#(0xd0 | 0xf)	/* enter system	*/
msr	cpsr_c,	#(0xd0 | 0x0)	/* enter usr	*/
msr	cpsr_c,	#(0xd0 | 0x2)	/* enter irq	*/
msr	cpsr_c,	#(0xd0 | 0x1)	/* enter fiq	*/
msr	cpsr_c,	#(0xd0 | 0x7)	/* enter abort	*/
msr	cpsr_c,	#(0xd0 | 0xd)	/* enter undef	*/


.code 16
str	r0,	[r0]
mov	r0,	#1
add	r0,	r15,	#0
mov	r0,	#0
bx	pc
