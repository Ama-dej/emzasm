__CFLAGS=-Wall $(CFLAGS)

emzasm: src/main.c
	mkdir -p bin
	cc src/main.c -o bin/emzasm $(__CFLAGS)
