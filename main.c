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
	INTEGER,
	MNEMONIC,
	LABEL,
	LABEL_REFERENCE,
	PLUS,
	MINUS,
	STAR,
	SLASH,
	LPARENTHESIS,
	RPARENTHESIS
};

struct token_t {
	enum type_t type;
	int id;
};

struct symbol_t {
	enum type_t type;
	char *name;
	int offset;
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

int find_sublabel(char *s, int offset) // NE DELA PRAVILNO!!!!!!!!
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

const enum type_t sisymbolst[] = {PLUS, MINUS, STAR, SLASH, LPARENTHESIS, RPARENTHESIS};
const char sisymbols[] = {'+', '-', '*', '/', '(', ')'};
#define SISYMBOLS_LEN (sizeof(sisymbols) / sizeof(sisymbols[0]))

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
			parent = i;
			cmp = symbol_table[i].name;
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

int hextoint(char *s) 
{
	int num = 0;
	int i = 2;

	for (; s[i] != 0; i++) {
		char tlc = tolower(s[i]);

		if (isdigit(tlc)) {
			num <<= 4;
			num += tlc - '0';
		} else if (tlc >= 'a' && tlc <= 'f') {
			num <<= 4;
			num += tlc - 'a' + 10;
		} else {
			break;
		}
	}

	if (i == 2)
		return -1;

	return num;
}

int bintoint(char *s) 
{
	char c;
	int num = 0;
	int i = 2;

	for (; (c = s[i]) != 0; i++) {
		if (c == '0' || c == '1') {
			num <<= 1;
			num += c - '0';
		} else {
			break;
		}
	}

	if (i == 2)
		return -1;

	return num;
}

int dectoint(char *s) 
{
	char c;
	int num = 0;
	int i = 0;

	for (; (c = s[i]) != 0; i++) {
		if (isdigit(c)) {
			num *= 10;
			num += c - '0';
		} else {
			break;
		}
	}

	if (i == 2)
		return -1;

	return num;
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
	int cur_byte = 0;

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
			line++;
			column = 0;	
			second = false;
		} else if (c == ';') {
			do {
				c = fgetc(f);
			} while (c != '\n' && c != EOF);

			printf("\n");
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
				printf("LABEL[%s %d] ", buffer, cur_byte);
				t.type = LABEL;
				t.id = symbol_index;

				struct symbol_t sym = {LABEL, buffer, -1, -1};	

				if (buffer[0] == '.') {
					if (cur_parent == -1) {
						printf("Sublabel \"%s:\" on line %d has no parent label.\n", buffer, line);
						free(buffer);
						fclose(f);
						return -1;
					}

					sym.parent = cur_parent;
				} else {
					cur_parent = symbol_index;
				}

				sym.offset = cur_byte;
				symbol_table[symbol_index++] = sym;
			} else if (!second) {
				printf("MNEMONIC[%s %d] ", buffer, ismnemonic(buffer));
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

				cur_byte++;
				free(buffer);
			} else {
				printf("REFERENCE[%s] ", buffer);
				t.type = LABEL_REFERENCE;
				t.id = symbol_index;
				skip = true;

				struct symbol_t sym = {LABEL_REFERENCE, buffer, -1};
				symbol_table[symbol_index++] = sym;
			}

			column += strlen(buffer) - 1;
		} else if (isdigit(c)) {
			char *buffer = malloc(256);

			int i = 0;

			do {
				buffer[i++] = c;
				c = tolower(fgetc(f));
			} while (isdigit(c) || ((i == 1) && (c == 'x' || c == 'b')));	// TODO: hmm več črk je loh not...
			
			buffer[i] = 0;
			
			i = 0;
			int num = 0;

			if (buffer[0] == '0' && buffer[1] == 'x') {
				num = hextoint(buffer);
			} else if (buffer[0] == '0' && buffer[1] == 'b') {
				num = bintoint(buffer);
			} else {
				num = dectoint(buffer);
			}

			if (num == -1) {
				printf("Invalid character or syntax for value \"%s\" on line %d, column %d.\n", buffer, line, column);
				free(buffer);
				fclose(f);
				return -1;
			}

			printf("INTEGER[%d] ", num);

			t.type = INTEGER;
			t.id = num;

			column += strlen(buffer) - 1;
			skip = true;
			free(buffer);
		} else {
			int i = 0;
			bool found = false;

			for (; i < SISYMBOLS_LEN; i++) {
				if (c == sisymbols[i]) {
					found = true;
					break;		
				}
			}

			if (!found) {
				printf("Unrecognized symbol \'%c\' on line %d, column %d.\n", c, line, column);
				fclose(f);
				return -1;
			}

			t.type = sisymbolst[i];

			printf("[%c] ", sisymbols[i]);
		}
	}

	/*printf("Illegal symbol \"%s\" on line %d, column %d.", buffer, line, column);
	fclose(f);
	return -1;*/
	fclose(f);
	return 0;
}
