; interrupt routines and space for stack

.global _start, _err, _timer, _terminal
.extern main, cout, timerCount, timer_cfg, inputCount

.section text:
	_start:
		mov $stack, %r6		; init the stack pointer
		add $256, %r6
		push $main			; entry point for the program - will be written into PC when this routine exits
		push $0				; init PSW
		mov $0, timer_cfg	; init timer_cfg - 500ms cycle
		iret
	_err:
		mov $error_message_start, %r0
	loop_start:
		mov (%r0), %r1
		call cout
		cmp %r0, $error_message_end
		jeq loop_end
		add $2, %r0
		jmp loop_start
	loop_end:
		halt
	error_message_start:
		.word 10, 69, 82, 82, 79, 82, 32, 66, 65, 68, 32, 79, 80, 33
	error_message_end:
		.word 10
	_timer:
		add $1, timerCount
		iret
	_terminal:
		add $1, inputCount
		iret

.section stack:
	.skip 256

.end