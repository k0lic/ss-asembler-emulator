; Memory Mapped Registers - this section should be placed at 0xff00
.global data_out, data_in, timer_cfg
.section mmr:
	data_out:
		.skip 2
	data_in:
		.skip 2
		.skip 12
	timer_cfg:
		.skip 2
		.skip 238
.end