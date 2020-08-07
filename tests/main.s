; main program

.global main, cout, timerCount, inputCount
.extern data_out, data_in, timer_cfg

.section text:
	main:
		mov $hello_msg, %r1
		call cout_string
		call endl
		;.byte 13, 14, 15, 16, 17, 18, 19	; remove ';' at start of the line to trigger illegal instruction interrupt

		mov $input_operand_msg, %r1
		call cout_string
		mov $97, %r1
		call cout
		mov $32, %r1
		call cout
		call read_number
		call endl
		cmp %r0, $0
		jne bad_read
		mov %r1, %r2

		mov $input_operand_msg, %r1
		call cout_string
		mov $98, %r1
		call cout
		mov $32, %r1
		call cout
		call read_number
		call endl
		test %r0, %r0
		jne bad_read
		mov %r1, %r3

		call endl
		mov %r2, %r1
		call cout_number
		mov $plus_msg, %r1
		call cout_string
		mov %r3, %r1
		call cout_number
		mov $equals_msg, %r1
		call cout_string
		mov %r2, %r1
		add %r3, %r1
		call cout_number
		call endl

		call endl
		mov $bye_msg, %r1
		call cout_string
		mov timerCount, %r1
		call cout_number
		call endl
	end_program:
		halt
		.equ bad_read_message_end, 22 + bad_read_message_start - 2
	bad_read_message_start:
		.word 10, 66, 65, 68, 32, 82, 69, 65, 68, 33, 10
	bad_read:
		mov $bad_read_message_start, %r0
		mov $bad_read, %r2
		add $15, %r2
		; %r2 should point at next instruction
		mov (%r0), %r1
		call cout
		cmp %r0, $bad_read_message_end
		jeq end_program
		add $2, %r0
		jmp *%r2
	cout:					; prints the char from %r1
		push %r0
		mov $data_out, %r0
		mov %r1, (%r0)
		pop %r0
		ret
	cout_string:			; prints the string from mem[%r1]
		cmp (%r1), $0
		jeq string_end
		push %r1
		mov (%r1), %r1
		call cout
		pop %r1
		add $2, %r1
		jmp cout_string
	string_end:
		ret
	cout_number:			; prints the number from %r1
		push %r2
		push %r3
		mov %r1, %r2
		mov %r6, %r3
	loop1:
		mov %r2, %r1
		call mod10
		push %r1
		div $10, %r2
		jne loop1
	loop2:
		cmp %r6, %r3
		jeq done
		pop %r1
		add $48, %r1
		call cout
		jmp loop2
	done:
		pop %r3
		pop %r2
		ret
	endl:					; prints a new line char '\n'
		push %r1
		mov $10, %r1
		call cout
		pop %r1
		ret
	mod10:					; %r1 = %r1 mod 10
		push %r2
		mov %r1, %r2
		div $10, %r2
		mul $10, %r2
		sub %r2, %r1
		pop %r2
		ret
	read_char:				; waits for input - returns it in %r1
		push %r0
		mov inputCount, %r0
	wait:
		cmp inputCount, %r0
		jeq wait
		mov data_in, %r1
		pop %r0
		ret
	read_number:			; tries to read a positive integer from terminal - %r0 is 0 if successful, 1 if failed - %r1 contains the number
		push %r2
		push %r3
		push %r4
		sub %r2, %r2
	one_more_char:
		call read_char
		call cout
		cmp $47, %r1
		jgt next_check
	NaN:
		cmp $10, %r1
		jeq whitespace
		cmp $32, %r1
		jeq whitespace
	number_failed:
		mov $1, %r0
		pop %r4
		pop %r3
		pop %r2
		ret
	next_check:
		cmp $57, %r1
		jgt NaN
		add $-48, %r1
		movb %r1l, buffer(%r2)
		add $1, %r2
		cmp %r2, $5
		jeq number_failed
		jmp one_more_char
	whitespace:
		cmp $0, %r2
		jeq number_failed
		mov $0, %r1
		mov $1, %r4
	combine_digits:
		sub $1, %r2
		movb $0, %r3h
		movb buffer(%r2), %r3l
		mul %r4, %r3
		add %r3, %r1
		mul $10, %r4
		cmp $0, %r2
		jne combine_digits
		mov $0, %r0
		pop %r4
		pop %r3
		pop %r2
		ret

.section data:
	timerCount:
		.word 0
		.word 0
	buffer:
		.skip 20
	hello_msg:
		.word 72, 101, 108, 108, 111, 33, 32, 73, 32, 99, 97, 110, 32, 99, 97, 108, 99, 117, 108, 97, 116, 101, 58, 9, 97, 32, 43, 32, 98, 0
	input_operand_msg:
		.word 80, 108, 101, 97, 115, 101, 32, 105, 110, 112, 117, 116, 32, 0
	plus_msg:
		.word 32, 43, 32, 0
	equals_msg:
		.word 32, 61, 32, 0
	bye_msg:
		.word 84, 105, 109, 101, 114, 32, 99, 111, 117, 110, 116, 58, 32, 0
		.equ inputCount, timerCount + 2

.end