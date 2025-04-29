start:
	lbz 0
	lab

loop:
	adis 1
	jmp start

	xci 0
	lam 0
	jmp loop
