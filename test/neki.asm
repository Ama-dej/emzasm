oznaka:	
	rt
	JMS oznaka
.pod:

	jmp .pod ; To je super duper kul komentar.

neki:
.pod:
	jmp (((2<<3)*7/3)&~0b111111100)+100+(oznaka.pod&0b11)

	rt

fasf:
	jms neki.pod
	pp 0
	jmp .trololo * 9999 + 1 / 1 % 2 - $
	dd ~0b111
	db ~$
	db 222
	$$
	.org 23
	$$
	;times 21 db 1
.trololo:
	times 10 - 5 * 2 + 2 dd 0x10 / 4 + hehehe

hehehe:
