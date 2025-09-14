const char sisymbols[] = {'+', '-', '*', '/', '%', '(', ')', '&', '|', '^', '~'};
const enum type_t sisymbolst[] = {PLUS, MINUS, STAR, SLASH, REMAINDER, LPARENTHESIS, RPARENTHESIS, AND, OR, XOR, NOT};
#define SISYMBOLS_LEN (int)(sizeof(sisymbols) / sizeof(sisymbols[0]))

const char *directives[] = {"db", "dw", "dd", ".org", "times"};
const enum type_t directivest[] = {DX_DIRECTIVE, DX_DIRECTIVE, DX_DIRECTIVE, ORG_DIRECTIVE, TIMES_DIRECTIVE};
#define DIRECTIVES_LEN (int)(sizeof(directivest) / sizeof(directivest[0]))

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

int isdirective(char *s) // hm, funkcija sumljivo podobna fuknciji ismnemonic...
{
	for (int i = 0; i < DIRECTIVES_LEN; i++) {
		for (int i = 0; s[i] != 0; s[i] = tolower(s[i]), i++);

		if (strcmp(s, directives[i]) == 0)
			return i;
	}

	return -1;
}

int lex(FILE *f)
{
	int t_len = 0;
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
			DEBUG_PRINT(("\n"));
			line++;
			column = 0;	
			second = false;
		} else if (c == ';') {
			do {
				c = fgetc(f);
			} while (c != '\n' && c != EOF);

			DEBUG_PRINT(("\n"));
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
			int di;

			if (c == ':') { // je 100 % oznaka
				DEBUG_PRINT(("LABEL[%s %d] ", buffer, cur_byte));
				t.type = LABEL;
				t.id = symbol_index;

				struct symbol_t sym = {LABEL, buffer, -1, -1};	

				if (buffer[0] == '.') {
					if (cur_parent == -1) {
						printf("Sublabel \"%s:\" on line %d has no parent label.\n", buffer, line);
						quit(f, NULL, buffer);
					} else if (find_sublabel(buffer, cur_parent + 1) != -1) {
						printf("Duplicate sublabel \"%s:\" on line %d, column %d.\n", buffer, line, column);
						quit(f, NULL, buffer);
					}

					printf("{%d} ", cur_parent);
					sym.parent = cur_parent;
				} else {
					if (find_label(buffer, 0) != -1) {
						printf("Duplicate label \"%s:\" on line %d, column %d.\n", buffer, line, column);
						quit(f, NULL, buffer);
					}

					cur_parent = symbol_index;
				}

				//sym.offset = cur_byte;
				symbol_table[symbol_index++] = sym;
			} else if ((di = isdirective(buffer)) != -1 && !second){
				DEBUG_PRINT(("DIRECTIVE[%s] ", buffer));
				t.type = directivest[di];
				t.id = di;
				skip = true;
				
				if (t.type != TIMES_DIRECTIVE)
					second = true;

				cur_byte++;
				free(buffer);
			} else if (!second) {
				DEBUG_PRINT(("MNEMONIC[%s %d] ", buffer, ismnemonic(buffer)));
				t.type = MNEMONIC;
				t.id = ismnemonic(buffer);
				second = true;
				skip = true;

				if (t.id == -1) {
					printf("Illegal instruction \"%s\" on line %d, column %d.\n", buffer, line, column);
					quit(f, NULL, buffer);
				}

				cur_byte++;
				free(buffer);
			} else {
				DEBUG_PRINT(("REFERENCE[%s] ", buffer));
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
				quit(f, NULL, buffer);
			}

			DEBUG_PRINT(("INTEGER[%d] ", num));

			t.type = INTEGER;
			t.id = num;

			column += strlen(buffer) - 1;
			skip = true;
			free(buffer);
		} else if (c == '<') {
			column++;

			if ((c = fgetc(f)) == '<') {
				t.type = LEFT_SHIFT;
				DEBUG_PRINT(("[<<] "));
			} else {
				printf("Unrecognized symbol \'%c\' on line %d, column %d.\n", c, line, column);
				quit(f, NULL, NULL);
			}
		} else if (c == '>') {
			column++;

			if ((c = fgetc(f)) == '>') {
				t.type = RIGHT_SHIFT;
				DEBUG_PRINT(("[>>] "));
			} else {
				printf("Unexpected symbol on line %d, column %d. Expected \'>\' but got \'%c\'.\n", line, column, c);
				quit(f, NULL, NULL);
			}
		} else if (c == '$') {
			column++;

			if ((c = fgetc(f)) == '$') {
				t.type = CURRENT_SEGMENT;
				DEBUG_PRINT(("[$$] "));
			} else {
				t.type = CURRENT_ADDRESS;
				skip = true;
				DEBUG_PRINT(("[$] "));
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
				quit(f, NULL, NULL);
			}

			t.type = sisymbolst[i];

			printf("[%c] ", sisymbols[i]);
		}
		
		tokens[token_index++] = t;
	}

	t_len = token_index;

	if (tokens[t_len - 1].type != NEWLINE)
		tokens[t_len++].type = NEWLINE;

	return t_len;
}
