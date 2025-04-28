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

struct opcode_t {
    uint8_t opcode;
    uint8_t argument_mask;
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
