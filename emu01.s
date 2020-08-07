; ovaj program ispisuje engleski alfabet
.global data_out, data_in, timer_cfg
.global main, cout
.equ b, a + 2

.section ivt:
		.word _start
		.skip 14

.section text:
	_start:
		mov $sp_ini, %r6 		; inicijalizuj SP
		add $1, %r6
		push $main				; gurni ulazna tacka programa na stek kako bi je pokupio PC
		push $0					; inicijalna vrednost za PSW
		mov $0, timer_cfg
		iret					; zavrsi pocetnu prekidnu rutinu i kreni sa programom
		.skip 10
	main:
		mov $64, a
		mov $90, b
	loop:
		add $1, a
		mov a, %r1
		call cout
		cmp b, a
		jne loop
		mov $10, %r1
		call cout
		halt
	cout:
		push %r0
		mov $data_out, %r0
		mov %r1, (%r0)
		pop %r0
		ret

.section bss:
	a:	.skip 2
		.skip 2

.section stack:
		.skip 255
	sp_ini:
		.skip 1

.section mmr:
	data_out:
		.skip 2
	data_in:
		.skip 2
		.skip 12
	timer_cfg:
		.skip 2

.end