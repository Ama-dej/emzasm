oznaka:	rt
	NOP
	RT
	AND
	LAE
	XAE
	INP
	XOR
	JMS oznaka
	0x10	
	0b1010*(1-0x2+3)
.pod:

	jmp .pod ; To je super duper kul komentar.

neki:
.pod:
	jmp oznaka.pod

fasf:
	jmp .pod
	jms neki.pod
