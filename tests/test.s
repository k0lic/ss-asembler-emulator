.global _start, asdf, a, b, x
.extern f_var1, f_var2, fun
.section text:
	_start:									;ulazna tacka programa
		mov var_x1, a
		mov var_x2, b
		movw $a, %r0
		.equ c, var_x2 - 2 - b + a
		sub b, a
		jeq labela1
		jgt labela2
		jmp labela3
		movw $7, b							;nikad se ne izvrsava
	labela2:
		mov $x, %r0
		movb (%r0), %r1l
		movb 3(%r0), %r1h
		xor a(%r0), c
	labela1:
		sub $1, a
		cmp $3, a
		jgt opet
		jmp kraj
		jmp 23
		jmp *%r0
		jmp *(%r1)
		jmp *11(%r6)
		jmp *a(%r7)
		jmp *b(%pc)
		jmp *89
		jmp *%psw
	asdf:
		jmp asdf
		movb $hundred, %r4h
	kraj:
		push $100
		push $50
		call fun
		pop %r0
		pop %r0								; %r0 = 150 ???
		add %r0, f_var2
		halt
		.equ labela3, labela2 + var_x2 - var_x1 - 1
		.equ opet, _start + 19
.section data:
	var_x1:
		.word 7, 13, b
	var_x2:
		.word 3, 65535
		.byte 1, 1, 2, 3, 5, 8, 13, 21
		jmp asdf
		cmp $8, $-4
.section bss:
	a:	.skip 2
	b:	.skip 2
	x:	.skip 100
		.equ onefifty, fifty + hundred - 0
		.equ hundred, 39 + fifty + 92 - 18 + 49 - 62 - 50
		.equ fifty, 1 + 50 - 1
.end