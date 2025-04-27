testek:
oznaka:	rt
	NOP
	RT
	AND
	LAE
	XAE
	INP
	XOR
	JMS oznaka
	(1)*(2)
	0x10>>1+~10
	0b1010*(-1-0x2+3<<1)
.pod:

	jmp .pod ; To je super duper kul komentar.

neki:
.pod:
	jmp (((2<<3)*7/3)&~0b111111100)+100+(oznaka.pod&0b11)

	rt

fasf:
	jms neki.pod
	jmp .trololo
.trololo:
