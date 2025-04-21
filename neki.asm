oznaka:	rt
	NOP
	RT
	AND
	LAE
	XAE
	INP
	XOR
	JMS oznaka
.pod:

neki:
.pod:
	jmp oznaka.pod

fasf:
	jmp .pod
	jms neki.pod
