#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lexer.h"
#include "instructions.h"
#include "tokens.h"

#define INSTRUCTION_NUMBER 76

typedef struct {
    char text[8];
    InstructionType type;
    int length;
} Instr;

Instr instrs[] = {
        {"abs", I_ABS, 3},
        {"add", I_ADD, 3},
        {"addi", I_ADDI, 4},
        {"addu", I_ADDU, 4},
        {"addiu", I_ADDIU, 5},
        {"and", I_AND, 3},
        {"andi", I_ANDI, 4},
        {"div", I_DIV, 3},
        {"divu", I_DIVU, 4},
        {"mult", I_MULT, 3},
        {"multu", I_MULTU, 4},
        {"neg", I_NEG, 3},
        {"negu", I_NEGU, 4},
        {"nor", I_NOR, 3},
        {"not", I_NOT, 3},
        {"or", I_OR, 2},
        {"rem", I_REM, 3},
        {"remu", I_REMU, 4},
        {"sll", I_SLL, 3},
        {"sllv", I_SLLV, 4},
        {"sra", I_SRA, 3},
        {"srav", I_SRAV, 4},
        {"srl", I_SRL, 3},
        {"srlv", I_SRLV, 4},
        {"sub", I_SUB, 3},
        {"subu", I_SUBU, 4},
        {"xor", I_XOR, 3},
        {"xori", I_XORI, 4},
        {"li", I_LI, 2},
        {"slt", I_SLT, 3},
        {"sltu", I_SLTU, 4},
        {"slti", I_SLTI, 4},
        {"sltiu", I_SLTIU, 5},
        {"seq", I_SEQ, 3},
        {"sge", I_SGE, 3},
        {"sgeu", I_SGEU, 4},
        {"sgt", I_SGT, 3},
        {"sgtu", I_SGTU, 4},
        {"sle", I_SLE, 3},
        {"sleu", I_SLEU, 4},
        {"sne", I_SNE, 3},
        {"b", I_B, 1},
        {"beq", I_BEQ, 3},
        {"bgez", I_BGEZ, 4},
        {"bgtz", I_BGTZ, 4},
        {"blez", I_BLEZ, 4},
        {"bltz", I_BLTZ, 4},
        {"bne", I_BNE, 3},
        {"beqz", I_BEQZ, 4},
        {"bge", I_BGE, 3},
        {"bgeu", I_BGEU, 4},
        {"bgt", I_BGT, 3},
        {"bgtu", I_BGTU, 4},
        {"ble", I_BLE, 3},
        {"bleu", I_BLEU, 4},
        {"blt", I_BLT, 3},
        {"bltu", I_BLTU, 4},
        {"bnez", I_BNEZ, 4},
        {"j", I_J, 1},
        {"jal", I_JAL, 3},
        {"jalr", I_JALR, 4},
        {"jr", I_JR, 2},
        {"la", I_LA, 2},
        {"lb", I_LB, 2},
        {"lbu", I_LBU, 3},
        {"lh", I_LH, 2},
        {"lhu", I_LHU, 3},
        {"lw", I_LW, 2},
        {"sb", I_SB, 2},
        {"sh", I_SH, 2},
        {"move", I_MOVE, 4},
        {"mfhi", I_MFHI, 4},
        {"mflo", I_MFLO, 4},
        {"mthi", I_MTHI, 4},
        {"mtlo", I_MTLO, 4},
        {"syscall", I_SYSCALL, 7}
};

// Function declarations
char peek(Lexer* lexer);
char advance(Lexer* lexer);
bool isAtEnd(Lexer* lexer);
void skipWhitespace(Lexer*);
Token getNextToken(Lexer*);
Token makeToken(TokenType type, Lexer* lexer);
Token getScalar(Lexer* lexer, char c);
Token checkInstruction(Lexer* lexer);
Token getIdentifier(Lexer* lexer);
Token getDirective(Lexer* lexer);
Token getRegister(Lexer* lexer);
Token getString(Lexer* lexer);
void reportError(const Token* token);
bool matches(const char* given, const char* expected, int count);

// Functions implementations
void resetLexer(Lexer* lexer) {
    lexer->source = NULL;
    lexer->current = NULL;
    lexer->start = NULL;
    lexer->state = STATE_OK;
    lexer->line = 1;
}

void initLexer(Lexer* lexer) {
    resetLexer(lexer);
    TokenList list;

    initTokenList(&list);
    lexer->tokens = list;
}

void freeLexer(Lexer* lexer) {
    freeTokenList(&lexer->tokens);
    resetLexer(lexer);
}

void tokenize(Lexer* lexer, const char* program) {
    if (program == NULL) {
        fprintf(stderr, "Cannot tokenize null input.");
        exit(75);
    }

    lexer->start = program;
    lexer->source = program;
    lexer->current = program;

    while (true) {
        Token token = getNextToken(lexer);
        addToken(&lexer->tokens, token);
        if (token.type == T_ERROR) {
            reportError(&token);
            lexer->state = STATE_ERROR;
        }
        if (token.type == T_EOF) break;
    }
}

Token getNextToken(Lexer* lexer) {
    skipWhitespace(lexer);
    lexer->start = lexer->current;
    if (isAtEnd(lexer)) return makeToken(T_EOF, lexer);

    char c = advance(lexer);
    if (isAlpha(c)) return getIdentifier(lexer);
    if (isDigit(c)) return getScalar(lexer, c);

    switch (c) {
        case ',':
            return makeToken(T_COMMA, lexer);
        case '(':
            return makeToken(T_LPAREN, lexer);
        case ')':
            return makeToken(T_RPAREN, lexer);
        case '.':
            return getDirective(lexer);
        case '$':
            return getRegister(lexer);
        case '"':
            return getString(lexer);
        default:
            return makeToken(T_ERROR, lexer);
    }
}

Token getIdentifier(Lexer* lexer) {
    TokenType type = T_IDENTIFIER;
    while (isAlphaNum(peek(lexer))) {
        advance(lexer);
    }

    if (peek(lexer) == ':') {
        advance(lexer);
        type = T_LABEL;
    }

    return type == T_LABEL ? makeToken(type, lexer) : checkInstruction(lexer);
}

Token checkInstruction(Lexer* lexer) {
    for (int i = 0; i < INSTRUCTION_NUMBER; ++i) {
        if (matches(lexer->start, instrs[i].text, instrs[i].length)) {
            Token token = makeToken(T_INSTRUCTION, lexer);
            token.value.type = instrs[i].type;
            return token;
        }
    }

    return makeToken(T_IDENTIFIER, lexer);
}

Token* isDirective(Token* token, const char* text, int length, DirectiveType type) {
    if (matches(token->start, text, length)) {
        token->value.type = type;
    } else {
        token->type = T_ERROR;
        fprintf(stderr, "Unrecognized directive %.*s\n", token->length, token->start);
    }

    return token;
}

Token getDirective(Lexer* lexer) {
    while (isAlpha(peek(lexer))) {
        advance(lexer);
    }

    Token token = makeToken(T_DIRECTIVE, lexer);
    switch (*(lexer->start + 1)) {
        case 'a': {
            if (matches(token.start, ".asciiz", 7)) {
                token.value.type = D_ASCII;
                return token;
            } else if (matches(token.start, ".ascii", 6)) {
                token.value.type = D_ASCIIZ;
                return token;
            }
            break;
        }
        case 'b':
            return *isDirective(&token, ".byte", 5, D_BYTE);
        case 'd':
            return *isDirective(&token, ".data", 5, D_DATA);
        case 'h':
            return *isDirective(&token, ".half", 5, D_HALF);
        case 's':
            return *isDirective(&token, ".space", 6, D_SPACE);
        case 't':
            return *isDirective(&token, ".text", 5, D_TEXT);
        case 'w':
            return *isDirective(&token, ".word", 5, D_WORD);
    }

    token.type = T_ERROR;
    fprintf(stderr, "Unrecognized directive %.*s\n", token.length, token.start);
    return token;
}

Token getRegister(Lexer* lexer) {
    while (isAlphaNum(peek(lexer))) {
        advance(lexer);
    }

    return makeToken(T_REGISTER, lexer);
}

Token getString(Lexer* lexer) {
    char* buffer = malloc(sizeof(char));
    int idx = 0;

    while (peek(lexer) != '"' && !isAtEnd(lexer)) {
        char c = advance(lexer);
        if (c == '\\') {
            switch (peek(lexer)) {
                case 'a':
                    c = '\a'; advance(lexer);
                    break;
                case 'b':
                    c = '\b'; advance(lexer);
                    break;
                case 'f':
                    c = '\f'; advance(lexer);
                    break;
                case 'n':
                    c = '\n'; advance(lexer);
                    break;
                case 'r':
                    c = '\r'; advance(lexer);
                    break;
                case 't':
                    c = '\t'; advance(lexer);
                    break;
                case 'v':
                    c = '\v'; advance(lexer);
                    break;
                case '"':
                    c = '"'; advance(lexer);
                    break;
                case '\\':
                    c = '\\'; advance(lexer);
                    break;
                case '\'':
                    c = '\''; advance(lexer);
                    break;
            }
        }
        buffer[idx] = c;
        idx++;
    }

    if (isAtEnd(lexer)) {
        free(buffer);
        fprintf(stderr, "Unterminated string at line %d\n", lexer->line);
        exit(75);
    }

    // Consume last "
    advance(lexer);
    buffer[idx] = '\0';

    Token token = makeToken(T_STRING, lexer);
    // Init string value pointer
    token.value.string = malloc(idx * sizeof(char));
    token.value.string = strcpy(token.value.string, buffer);
    free(buffer);

    return token;
}

Token getScalar(Lexer* lexer, char c) {
    Token token;
    int32_t value = 0;
    // Not a decimal number
    if (c == '0') {
        char nxt = peek(lexer);
        if(nxt == 'x' || nxt == 'X') {
            // Parse hexadecimal
            advance(lexer);
            while (isHexDigit(peek(lexer))) advance(lexer);
            value = strtoul(lexer->start, NULL, 16);
        } else {
            // Parse octal
            while (isOctDigit(peek(lexer))) advance(lexer);
            value = strtoul(lexer->start + 1, NULL, 8);
        }
    } else {
        while (isDigit(peek(lexer))) advance(lexer);
        value = strtoul(lexer->start, NULL, 10);
    }

    token = makeToken(T_SCALAR, lexer);
    token.value.scalar = value;

    return token;
}

void skipWhitespace(Lexer* lexer) {
    while (true) {
        char c = peek(lexer);

        switch (c) {
            case '\n':
                lexer->line++;
            case ' ':
            case '\t':
            case '\r':
                advance(lexer);
                break;
            case '#': {
                advance(lexer);
                while (peek(lexer) != '\n' && !isAtEnd(lexer)) advance(lexer);
                break;
            }
            default:
                return;
        }
    }
}

Token makeToken(TokenType type, Lexer* lexer) {
    return (Token) {type, lexer->start, (int)(lexer->current - lexer->start), lexer->line};
}

bool isAtEnd(Lexer* lexer) {
    return *lexer->current == '\0';
}

char peek(Lexer* lexer) {
    return *lexer->current;
}

char advance(Lexer* lexer) {
    lexer->current++;
    return lexer->current[-1];
}

void reportError(const Token* token) {
    fprintf(stderr, "[line %d] Error at '%.*s' : Unexpected character.\n", token->line, token->length, token->start);
}

bool matches(const char* given, const char* expected, int count) {
    return strncmp(given, expected, count) == 0;
}