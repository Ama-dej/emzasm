#include <stdio.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

const char *mnemonics[] = {
	"nop", 
	"rt",
	"rts",
	"psh",
	"psl",
	"and",
	"sos",
	"sbe",
	"szc",
	"stc",
	"rsc",
	"lae",
	"xae",
	"inp",
	"eur",
	"cma",
	"xabu",
	"lab",
	"xab",
	"adcs",
	"xor",
	"add",
	"sam",
	"disb",
	"mvs",
	"out",
	"disn",
	"szm",
	"stm",
	"rsm",
	"szk",
	"szi",
	"rf1",
	"sf1",
	"rf2",
	"sf2",
	"tf1",
	"tf2",
	"xci",
	"xcd",
	"xc",
	"lam",
	"lbz",
	"lbf",
	"lbe",
	"lbep",
	"adis",
	"pp",
	"lai",
	"jms",
	"jmp",
	"\0"
};

enum type_t {
	NEWLINE,
	MNEMONIC,
	LABEL,
	LABEL_REFERENCE
};

struct token_t {
	enum type_t type;
	int id;
};

struct symbol_t {
	enum type_t type;
	char *name;
	int parent;
};

int ismnemonic(char *s)
{
	int i;

	for (i = 0; mnemonics[i][0] != '\0'; i++) {
		for (int i = 0; s[i] != 0; s[i] = tolower(s[i]), i++);

		if (strcmp(s, mnemonics[i]) == 0)
			return i;
	}

	return -1;
}

struct symbol_t symbol_table[256];
int symbol_index = 0;

int find_sublabel(char *s, int offset)
{
	for (int i = offset; i < symbol_index; i++) {
		if (symbol_table[i].type != LABEL)
			continue;

		if (symbol_table[i].name[0] != '.')
			break;

		if (strcmp(s, symbol_table[i].name) == 0)
			return i;
	}

	return -1;
}

int find_label(char *s, int offset)
{
	int parent = -1;
	char searched[512];
	strcpy(searched, s);	
	bool free_flag = false;

	for (int i = offset; i < symbol_index; i++) {
		if (symbol_table[i].type != LABEL)
			continue;

		char *cmp;

		if (symbol_table[i].name[0] == '.') {
			cmp = malloc(512);
			strcpy(cmp, symbol_table[parent].name);
			strcpy(cmp + strlen(cmp), symbol_table[i].name);
			free_flag = true;
		} else {
			cmp = symbol_table[i].name;
			parent = i;
		}

		printf("(%s %s)", cmp, searched);

		if (strcmp(searched, cmp) == 0) {
			if (free_flag)
				free(cmp);
			return i;
		}

		if (free_flag) {
			free(cmp);
			free_flag = false;
		}

		searched[strlen(s)] = 0;
	}

	return -1;
}

int main(int argc, char *argv[])
{
	FILE *f = fopen(argv[1], "r");

	if (f == NULL) {
		printf("An error occured whilst trying to open file \"%s\"", argv[1]);
		return -1;
	}

	char c;
	int line = 1;
	int column = 1;
	bool second = false;
	bool skip = false;
	int cur_parent = -1;

	for (;; column++) {
		if (!skip)
			c = fgetc(f);

		if (c == EOF)
			break;

		skip = false;
		struct token_t t = {NEWLINE, 0};

		if (c == ' ' || c == '\t') {
			continue;
		} else if (c == '\n') {
			printf("\n");
			t.type = NEWLINE;
			line++;
			column = 0;	
			second = false;
		} else if (isalpha(c) || c == '.') {
			char *buffer = malloc(256);

			int i = 0;

			do {
				buffer[i++] = c;
				c = fgetc(f);
			} while (isalpha(c) || isdigit(c) || c == '.' || c == '_' || c == '-');	// TODO: hmm več črk je loh not...

			buffer[i] = 0;

			if (c == ':') { // je 100 % oznaka
				printf("%s[LABEL] ", buffer);
				t.type = LABEL;
				t.id = symbol_index;

				struct symbol_t sym = {LABEL, buffer, -1};	

				if (buffer[0] == '.') {
					if (cur_parent == -1) {} // TODO: nared neki
					sym.parent = cur_parent;
				} else {
					cur_parent = symbol_index;
				}

				symbol_table[symbol_index++] = sym;
			} else if (!second) {
				printf("%s[MNEMONIC%d] ", buffer, ismnemonic(buffer));
				t.type = MNEMONIC;
				t.id = ismnemonic(buffer);
				second = true;
				skip = true;

				if (t.id == -1) {
					printf("Illegal instruction \"%s\" on line %d, column %d.", buffer, line, column);
					free(buffer);
					fclose(f);
					return -1;
				}

				free(buffer);
			} else {
				printf("%s[REFERENCE] ", buffer);
				t.type = LABEL_REFERENCE;
				t.id = symbol_index;
				skip = true;

				struct symbol_t sym = {LABEL_REFERENCE, buffer, -1};
				symbol_table[symbol_index++] = sym;

				/*
				if (buffer[0] == '.')
					printf("%d ", find_sublabel(buffer, symbol_index));
				else
					printf("%d ", find_label(buffer, 0));
				// ne tuki, ta del pride kasnej
				*/
			}

			column += strlen(buffer);
		}
	}

	/*printf("Illegal symbol \"%s\" on line %d, column %d.", buffer, line, column);
	fclose(f);
	return -1;*/
	fclose(f);
	return 0;
}
