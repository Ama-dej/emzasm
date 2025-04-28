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

int parse(FILE *wf, char *fn, int t_len)
{
	int starting_offset = 0;
	int offset = 0;
	int token_index = 0;
	int ptr0 = -1;
	int ptr1 = -1;
	int ptr2 = -1;
	int ptr3 = -1;
	int lookahead = -1;
	bool reset = false;
	bool mnemonic_present = false;
	int cur_size = 1;

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

		if (mnemonic_present != true) {
			if (t3.type == MNEMONIC || t3.type == DX_DIRECTIVE) {
				int size = 1;
	
				if (t3.type == DX_DIRECTIVE)
					size <<= t3.id;
	
				offset += size;
				cur_size = size;
				mnemonic_present = true;
			}
		} else if (t3.type == NEWLINE) {
			mnemonic_present = false;
			cur_size = 0;
		}

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
				remove(fn);
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
				remove(fn);
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
				remove(fn);
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
		} else if (t3.type == CURRENT_SEGMENT) {
			printf("[$$] -> [INT]\n");
			tokens[ptr3].type = INTEGER;
			tokens[ptr3].id = 0; // trenutno, rabm se še odločt kako nej bi to delval (mogoče id & (~63)?)
			reset = true;
		} else if (t3.type == CURRENT_ADDRESS) {
			tokens[ptr3].type = INTEGER;

			if (mnemonic_present)
				tokens[ptr3].id = offset - cur_size;
			else
				tokens[ptr3].id = offset;

			printf("[$] -> [INT] %d\n", tokens[ptr3].id);

			reset = true;
		} else if (t1.type == NONE && t2.type == DX_DIRECTIVE && t3.type == INTEGER && t_lookahead.type == NEWLINE) {
			int size = 1 << t2.id;
			starting_offset += size;

			printf("[DX%d] [INT] -> [NONE]\n", size);

			fwrite(&t3.id, size, 1, wf);

			tokens[ptr2].type = NONE;
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
			mnemonic_present = reset = false;
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
		printf("Invalid syntax for expression on line %d.\n", t.line, t.column);
		fclose(wf);
		remove(fn);
		return -1;
	}
}
