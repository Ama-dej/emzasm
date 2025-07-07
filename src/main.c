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
	memset(symbol_table, 0, sizeof(symbol_table));

	FILE *f = fopen(argv[1], "r");

	if (f == NULL) {
		printf("An error occured whilst trying to open file \"%s\"", argv[1]);
		return -1;
	}

	t_len = lex(f);

	fclose(f);
	
	char *fn = "a.bin";

	if (argc >= 3) {
		fn = argv[2];
	}

	FILE *wf = fopen(fn, "w+");
	parse(wf, fn, t_len);

	for (uint64_t i = 0; i < sizeof(symbol_table) / sizeof(symbol_table[0]); i++)
		free(symbol_table[i].name);

	fclose(wf);
	return 0;
}
