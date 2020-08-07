; Interrupt Vector Table - this section should be placed on the IVT address - which is 0x0000 in our case
.extern _start, _err, _timer, _terminal
.section ivt:
	.word _start
	.word _err
	.word _timer
	.word _terminal
	.skip 8
.end