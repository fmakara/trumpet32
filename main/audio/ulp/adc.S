
	.set adc_port, 0
	.set adc_mic_1, 0
	.set adc_mic_2, 1

	.set sample_frequency, 10000
	.set sample_buffer_size, 32
	.set sample_buffer_mask, sample_buffer_size - 1
	.set sample_buffer_size_bytes, sample_buffer_size * 4
	.set sample_wait_period, 700

	/* Define variables, which go into .bss section (zero-initialized data) */
	.bss

	.global adcreader_current_pos
adcreader_current_pos:
	.long 0

	.global adcreader_readings_mic1
adcreader_readings_mic1:
	.fill sample_buffer_size_bytes

	.global adcreader_readings_mic2
adcreader_readings_mic2:
	.fill sample_buffer_size_bytes

	/* Code goes into .text section */
	.text
	.global entry
entry:

measure:
	// Read both values (with minimum skew)
	adc r2, adc_port, adc_mic_1
	adc r3, adc_port, adc_mic_2

	// Move values to buffers
	move r0, adcreader_current_pos
	ld r1, r0, 0
	and r1, r1, sample_buffer_mask
	add r0, r1, adcreader_readings_mic1 //r0 = r1 (current_pos) + readings_1
	st r2, r0, 0
	add r0, r1, adcreader_readings_mic2
	st r3, r0, 0

	// Refresh the pointer
	move r1, adcreader_current_pos
	ld r0, r1, 0
	add r0, r0, 1
	st r0, r1, 0

	// Wait for next cycle
	wait sample_wait_period

	jump measure


