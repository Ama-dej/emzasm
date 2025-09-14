#ifdef DEBUG
#define DEBUG_PRINT(x) printf x
#else
#define DEBUG_PRINT(x) do {} while (0)
#endif

int find_sublabel(char *s, int offset)
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

void quit(FILE *f, char *name, void *buffer)
{
	fclose(f);
	remove(name);
	free(buffer);	

	for (uint64_t i = 0; i < sizeof(symbol_table) / sizeof(symbol_table[0]); i++)
        free(symbol_table[i].name);

	exit(-1);
}
