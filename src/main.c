#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

const char *mnemonics[] = {
	"nop", 
	"brk",
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

struct opcode_t {
	uint8_t opcode;
	uint8_t argument_mask;
};

const struct opcode_t opcodes[] = {
	{0b00000000, 0},
	{0b00000001, 0},
	{0b00000010, 0},
	{0b00000011, 0},
	{0b00000100, 0},
	{0b00000101, 0},
	{0b00000110, 0},
	{0b00000111, 0},
	{0b00001000, 0},
	{0b00001001, 0},
	{0b00001010, 0},
	{0b00001011, 0},
	{0b00001100, 0},
	{0b00001101, 0},
	{0b00001110, 0},
	{0b00001111, 0},
	{0b00010000, 0},
	{0b00010001, 0},
	{0b00010010, 0},
	{0b00010011, 0},
	{0b00010100, 0},
	{0b00010101, 0},
	{0b00010110, 0},
	{0b00010111, 0},
	{0b00011000, 0},
	{0b00011001, 0},
	{0b00011010, 0},
	{0b00011011, 0},
	{0b00011100, 0b11},
	{0b00100000, 0b11},
	{0b00100100, 0b11},
	{0b00101000, 0},
	{0b00101001, 0},
	{0b00101010, 0},
	{0b00101011, 0},
	{0b00101100, 0},
	{0b00101101, 0},
	{0b00101110, 0},
	{0b00101111, 0},
	{0b00110000, ~0b11},
	{0b00110100, ~0b11},
	{0b00111000, ~0b11},
	{0b00111100, ~0b11},
	{0b01000000, 0b11},
	{0b01000100, 0b11},
	{0b01001000, 0b11},
	{0b01001100, 0b11},
	{0b01010000, 0b1111},
	{0b01100000, ~0b1111},
	{0b01110000, 0b1111},
	{0b10000000, 0b111111},
	{0b11000000, 0b111111},
};

enum type_t {
	END,
	NONE,
	NEWLINE,
	INTEGER,
	MNEMONIC,
	LABEL,
	LABEL_LOC,
	LABEL_REFERENCE,
	PLUS,
	MINUS,
	STAR,
	SLASH,
	REMAINDER,
	LPARENTHESIS,
	RPARENTHESIS,
	AND,
	OR,
	XOR,
	NOT,
	LEFT_SHIFT,
	RIGHT_SHIFT
};

struct token_t {
	enum type_t type;
	int id;
	int line;
	int column;
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

int find_sublabel(char *s, int offset) // NE DELA PRAVILNO!!!!!!!! (dela pravilno)
{
	for (int i = offset; i < symbol_index; i++) {
		//printf("(%s) ", symbol_table[i].name);

		if (symbol_table[i].type != LABEL)
			continue;

		if (symbol_table[i].name[0] != '.')
			break;

		if (strcmp(s, symbol_table[i].name) == 0)
			return i;
	}

	return -1;
}

const enum type_t sisymbolst[] = {PLUS, MINUS, STAR, SLASH, REMAINDER, LPARENTHESIS, RPARENTHESIS, AND, OR, XOR, NOT};
const char sisymbols[] = {'+', '-', '*', '/', '%', '(', ')', '&', '|', '^', '~'};
#define SISYMBOLS_LEN (int)(sizeof(sisymbols) / sizeof(sisymbols[0]))

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

		//printf("(%s %s)", cmp, searched);

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

	return num;
}

struct token_t tokens[1 << 16];
int t_len = 0;

const enum type_t precedence_array[][4] = {
	{STAR, SLASH, REMAINDER, END},
	{PLUS, MINUS, END},
	{LEFT_SHIFT, RIGHT_SHIFT, END},
	{AND, END},
	{XOR, END},
	{OR, END}
};

#define PA_LEN (int)(sizeof(precedence_array) / sizeof(precedence_array[0]))

int next(int i)
{
	for (; i < t_len; i++) {
		if (tokens[i].type != NONE)
			return i;
	}

	return -1;
}

int precedence(enum type_t type)
{
	for (int i = 0; i < PA_LEN; i++) {
		for (int j = 0; precedence_array[i][j] != END; j++) {
			if (precedence_array[i][j] == type)
				return i;
		}
	}

	return PA_LEN;
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
	int token_index = 0;

	for (;; column++) {
		if (!skip)
			c = fgetc(f);

		if (c == EOF)
			break;

		skip = false;
		struct token_t t = {NEWLINE, 0, line, column};

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
					} else if (find_sublabel(buffer, cur_parent + 1) != -1) {
						printf("Duplicate sublabel \"%s:\" on line %d, column %d.\n", buffer, line, column);
						free(buffer);
						fclose(f);
						return -1;
					}

					printf("{%d} ", cur_parent);
					sym.parent = cur_parent;
				} else {
					if (find_label(buffer, 0) != -1) {
						printf("Duplicate label \"%s:\" on line %d, column %d.\n", buffer, line, column);
						free(buffer);
						fclose(f);
						return -1;
					}

					cur_parent = symbol_index;
				}

				//sym.offset = cur_byte;
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

				struct symbol_t sym = {LABEL_REFERENCE, buffer, -1, -1};
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
		} else if (c == '<') {
			column++;

			if ((c = fgetc(f)) == '<') {
				t.type = LEFT_SHIFT;
				printf("[<<] ");
			} else {
				printf("Unrecognized symbol \'%c\' on line %d, column %d.\n", c, line, column);
				fclose(f);
				return -1;
			}
		} else if (c == '>') {
			column++;

			if ((c = fgetc(f)) == '>') {
				t.type = RIGHT_SHIFT;
				printf("[>>] ");
			} else {
				printf("Unexpected symbol on line %d, column %d. Expected \'>\' but got \'%c\'.\n", line, column, c);
				fclose(f);
				return -1;
			}
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
				printf("Unexpected symbol on line %d, column %d. Expected \'<\' but got \'%c\'.\n", line, column, c);
				fclose(f);
				return -1;
			}

			t.type = sisymbolst[i];

			printf("[%c] ", sisymbols[i]);
		}
		
		tokens[token_index++] = t;
	}

	t_len = token_index;

	if (tokens[t_len - 1].type != NEWLINE)
		tokens[t_len++].type == NEWLINE;

	fclose(f);
	
	/*
	 * Grammar:
	 * [REFERENCE] -> [INT]
	 * [(] [INT] [)] -> [INT]
	 * [INT] [*] [INT] (PRECEDENCE) -> [INT]
	 * [INT] [/] [INT] (PRECEDENCE) -> [INT]
	 * [INT] [+] [INT] (PRECEDENCE) -> [INT]
	 * [INT] [-] [INT] (PRECEDENCE) -> [INT]
	 * [INT] [<<] [INT] (PRECEDENCE) -> [INT]
	 * [INT] [>>] [INT] (PRECEDENCE) -> [INT]
	 * [INT] [&] [INT] (PRECEDENCE) -> [INT]
	 * [INT] [|] [INT] (PRECEDENCE) -> [INT]
	 * [INT] [^] [INT] (PRECEDENCE) -> [INT]
	 * [+] [INT] -> [INT]
	 * [-] [INT] -> [INT]
	 * [~] [INT] -> [INT]
	 * (NONE) [MNEMONIC] [INT] (NEWLINE) || (NONE) [MNEMONIC] (NEWLINE) -> [NONE] //odvisno od inštrukcije
	 * (NONE) [LABEL] (NEWLINE | MNEMONIC) -> [NONE]
	 * (NONE) [NEWLINE] -> [NONE]
	 */

	char *fn = "a.bin";

	if (argc >= 3) {
		fn = argv[2];
	}

	FILE *wf = fopen(fn, "w+");

	int starting_offset = 0;
	int offset = 0;
	token_index = 0;
	int ptr0 = -1;
	int ptr1 = -1;
	int ptr2 = -1;
	int ptr3 = -1;
	int lookahead = -1;
	bool reset = false;

	while (true) {
		lookahead = next(lookahead + 1);
		int lookahead_precedence = precedence(tokens[lookahead].type);

		struct token_t t0 = {NONE, -1, -1, -1};
		struct token_t t1 = {NONE, -1, -1, -1};
		struct token_t t2 = {NONE, -1, -1, -1};
		struct token_t t3 = {NONE, -1, -1, -1};
		struct token_t t_lookahead = tokens[lookahead];
	
		if (ptr0 != -1)
			t0 = tokens[ptr0];	
		if (ptr1 != -1)
			t1 = tokens[ptr1];
		if (ptr2 != -1)
			t2 = tokens[ptr2];
		if (ptr3 != -1)
			t3 = tokens[ptr3];

		if (t3.type == MNEMONIC)
			offset++;

		if (t3.type == LABEL_REFERENCE) {
			struct symbol_t sym = symbol_table[t3.id];
			int pos;

			if (sym.name[0] == '.') {
				int i;
				for (i = t3.id; i >= 0; i--) {
					if (symbol_table[i].type == LABEL && symbol_table[i].name[0] != '.')
						break;
				}

				pos = find_sublabel(sym.name, i + 1);			
			} else {
				pos = find_label(sym.name, 0);
			}

			if (pos == -1) {
				printf("Invalid label \"%s:\".\n", sym.name);
				fclose(wf);
				remove(argv[2]);
				return -1;
			}

			if (symbol_table[pos].offset == -1) {
				printf("DEBUG: Label postition not yet known, skipping for now.\n");
			} else {
				//printf("%s, %d\n", sym.name, symbol_table[pos].offset);
				printf("[REFERENCE] -> [INT]\n");
			
				tokens[ptr3].type = INTEGER;
				tokens[ptr3].id = symbol_table[pos].offset;
				reset = true;
			}
		} else if (t1.type == LPARENTHESIS && t2.type == INTEGER && t3.type == RPARENTHESIS) {
			printf("[(] [INT] [)] -> [INT]\n");
			tokens[ptr1].type = NONE;
			tokens[ptr3].type = NONE;
			reset = true;
		} else if (t1.type == INTEGER && t3.type == INTEGER) {
			int t0_precedence = precedence(t0.type);

			if (t0_precedence > precedence(STAR) && t2.type == STAR && lookahead_precedence >= precedence(STAR)) {
				printf("[INT] [*] [INT] -> [INT]\n");
				tokens[ptr3].id = t1.id * t3.id;
				reset = true;
			} else if (t0_precedence > precedence(SLASH) && t2.type == SLASH && lookahead_precedence >= precedence(SLASH)) {
				printf("[INT] [/] [INT] -> [INT]\n");
				tokens[ptr3].id = t1.id / t3.id;
				reset = true;
			} else if (t0_precedence > precedence(REMAINDER) && t2.type == REMAINDER && lookahead_precedence >= precedence(REMAINDER)) {
				printf("[INT] [%] [INT] -> [INT]\n");
				tokens[ptr3].id = t1.id % t3.id;
				reset = true;
			} else if (t0_precedence > precedence(PLUS) && t2.type == PLUS && lookahead_precedence >= precedence(PLUS)) {
				printf("[INT] [+] [INT] -> [INT]\n");
				tokens[ptr3].id = t1.id + t3.id;
				reset = true;
			} else if (t0_precedence > precedence(MINUS) && t2.type == MINUS && lookahead_precedence >= precedence(MINUS)) {
				printf("[INT] [-] [INT] -> [INT]\n");
				tokens[ptr3].id = t1.id - t3.id;
				reset = true;
			} else if (t0_precedence > precedence(LEFT_SHIFT) && t2.type == LEFT_SHIFT && lookahead_precedence >= precedence(LEFT_SHIFT)) {
				printf("[INT] [<<] [INT] -> [INT]\n");
				tokens[ptr3].id = t1.id << t3.id;
				reset = true;
			} else if (t0_precedence > precedence(RIGHT_SHIFT) && t2.type == RIGHT_SHIFT && lookahead_precedence >= precedence(RIGHT_SHIFT)) {
				printf("[INT] [>>] [INT] -> [INT]\n");
				tokens[ptr3].id = t1.id >> t3.id;
				reset = true;
			} else if (t0_precedence > precedence(AND) && t2.type == AND && lookahead_precedence >= precedence(AND)) {
				printf("[INT] [&] [INT] -> [INT]\n");
				tokens[ptr3].id = t1.id & t3.id;
				reset = true;
			} else if (t0_precedence > precedence(OR) && t2.type == OR && lookahead_precedence >= precedence(OR)) {
				printf("[INT] [|] [INT] -> [INT]\n");
				tokens[ptr3].id = t1.id | t3.id;
				reset = true;
			} else if (t0_precedence > precedence(XOR) && t2.type == XOR && lookahead_precedence >= precedence(XOR)) {
				printf("[INT] [^] [INT] -> [INT]\n");
				tokens[ptr3].id = t1.id ^ t3.id;
				reset = true;
			}

			if (reset) {
				tokens[ptr1].type = NONE;
				tokens[ptr2].type = NONE;
			}
		} else if (t1.type != LABEL_REFERENCE && t2.type == PLUS && t3.type == INTEGER) {
			printf("[+] [INT] -> [INT]\n");
			tokens[ptr2].type = NONE;
			reset = true;
		} else if (t1.type != LABEL_REFERENCE && t2.type == MINUS && t3.type == INTEGER) {
			printf("[-] [INT] -> [INT]\n");
			tokens[ptr3].id = -t3.id;
			tokens[ptr2].type = NONE;
			reset = true;
		} else if (t2.type == NOT && t3.type == INTEGER) {
			printf("[~] [INT] -> [INT]\n");
			tokens[ptr3].id = ~t3.id;
			tokens[ptr2].type = NONE;
			reset = true;
		} else if (t1.type == NONE && t2.type == MNEMONIC && t3.type == INTEGER && t_lookahead.type == NEWLINE) {
			starting_offset++;
			int i = t2.id;			

			if (opcodes[i].argument_mask == 0) {
				printf("Instruction \"%s\" on line %d, column %d, expects no arguments, but arguments were given.\n", mnemonics[i], t2.line, t2.column);
				fclose(wf);
				remove(argv[2]);
				return -1;
			}

			uint8_t instruction;
			uint8_t mask;

			if ((opcodes[i].argument_mask & 1) == 0)
				mask = ~opcodes[i].argument_mask;
			else
				mask = opcodes[i].argument_mask;

			/*
			if (t3.id > mask)
				printf("Warning: Argument %d is bigger than expected (%d), truncating to %d.\n", t3.id, mask, t3.id & mask);
			*/
			
			instruction = opcodes[i].opcode | (t3.id & mask);	
			printf("[MNEMONIC] [INT] -> [NONE] %2Xh %d\n", instruction, t3.id);
			fwrite(&instruction, 1, 1, wf);

			tokens[ptr2].type = NONE;
			tokens[ptr3].type = NONE;
			reset = true;
		} else if (t2.type == NONE && t3.type == MNEMONIC && t_lookahead.type == NEWLINE) {
			starting_offset++;
			int i = t3.id;

			if (opcodes[i].argument_mask != 0) {
				printf("Instruction \"%s\" on line %d, column %d, expects one argument, but none were given.\n", mnemonics[i], t3.line, t3.column);
				fclose(wf);
				remove(argv[2]);
				return -1;
			}

			uint8_t instruction = opcodes[i].opcode;
			printf("[MNEMONIC] -> [NONE] %2Xh\n", instruction);
			fwrite(&instruction, 1, 1, wf);

			tokens[ptr3].type = NONE;
			reset = true;
		} else if (t2.type == NONE && t3.type == LABEL_LOC && (t_lookahead.type == NEWLINE || t_lookahead.type == MNEMONIC)) {
			printf("[LABEL_LOC] -> [NONE]\n");
			tokens[ptr3].type = NONE;
			reset = true;
		} else if (t3.type == LABEL) {	
			printf("[LABEL] -> [LABEL_LOC] %d\n", offset);
			symbol_table[tokens[ptr3].id].offset = offset;
			tokens[ptr3].type = LABEL_LOC;
			reset = true;
		} else if (t2.type == NONE && t3.type == NEWLINE) {
			printf("[NEWLINE] -> [NONE]\n");
			tokens[ptr3].type = NONE;
			reset = true;
		} else if (t2.type == NONE && t3.type == INTEGER && t_lookahead.type == NEWLINE) {
			printf("[INT] -> [NONE]\n");	
			tokens[ptr3].type = NONE;
			reset = true;
		}
		
		//printf("%d, %d\n", precedence(tokens[lookahead].type), precedence(PLUS));
		//printf("%d, %d, %d\n", ptr1, ptr2, ptr3);
		//printf("%d, %d, %d, %d\n", t1.type == INTEGER, t2.type == PLUS, t3.type == INTEGER, lookahead_precedence >= precedence(PLUS));
		
		if (reset) {
			ptr0 = ptr1 = ptr2 = ptr3 = -1;
			lookahead = next(0) - 1;
			offset = starting_offset;
			reset = false;
			continue;
		}

		ptr0 = ptr1;
		ptr1 = ptr2;
		ptr2 = ptr3;
		ptr3 = lookahead;

		if (lookahead == -1)
			break;
	}

	int tmp;

	if ((tmp = next(0)) != -1) {
		struct token_t t = tokens[tmp];
		printf("Invalid syntax on line %d.\n", t.line, t.column);
		fclose(wf);
		remove(argv[2]);
		return -1;
	}

	fclose(wf);
	return 0;
}
