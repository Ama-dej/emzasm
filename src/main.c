#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "typedef.h"
#include "globalvars.h"

#include "functions.c"
#include "lexer.c"
#include "parser.c"

int main(int argc, char *argv[])
{
	FILE *f = fopen(argv[1], "r");

	if (f == NULL) {
		printf("An error occured whilst trying to open file \"%s\"", argv[1]);
		return -1;
	}

	if ((t_len = lex(f)) < 0)
		return -1;

	fclose(f);
	
	char *fn = "a.bin";

	if (argc >= 3) {
		fn = argv[2];
	}

	FILE *wf = fopen(fn, "w+");

	if (parse(wf, fn, t_len) < 0)
		return -1;

	fclose(wf);
	return 0;
}
