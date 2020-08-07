.global f_var1, fun, f_var2
.extern asdf
.section data:
		.byte 2, 4, 5
	f_var1:
		.word 18
	f_var2:
		.word 500
		.word 750
.section haha:
		.word 10
.section text:
		push %r3
		pop	%r3
	fun:
		push %r0
		push %r1
		push %r2
		push %r3
		push %r4
		push %r5
		mov 14(%r6), %r0
		mov 16(%r6), %r1
		add %r1, %r0
		mov %r0, 16(%r6)
		pop %r5
		pop %r4
		pop %r3
		pop %r2
		pop %r1
		pop %r0
		ret
		jmp asdf
.end