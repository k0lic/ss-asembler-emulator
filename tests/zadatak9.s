.global a, c
.extern b
.section text:
		jeq a
		jeq e
		jeq b
		jeq d
	d:	.word d
		mov %r0, b
		mov c, %r0
		mov %r0, e
.section data:
		.skip 8
		.equ x, a - e + d
	e:	.word x
		.word c
		.word bss
	a:	.word b
.section bss:
	c:	.skip 8
.end