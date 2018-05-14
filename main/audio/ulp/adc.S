				.set sleep_reg, 0
				.bss
				.global m_count
m_count:		.long 0
				.text
				.global entry
entry:
				MOVE R0, m_count    //R0 = (&m_count)/4
				LD   R1, R0, 0		//R1 = *(R0*4+0)
				ADD  R1, R1, 1		//R1++
				ST   R1, R0, 0      //*(R0*4+0) = R1

				//WAIT 100

				HALT
