emzasm: src/main.c
	mkdir -p bin
	cc src/main.c -o bin/emzasm -Wall $(CFLAGS)
