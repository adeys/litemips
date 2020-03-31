#ifndef LMIPS_TOKENS
#define LMIPS_TOKENS

#include "common.h"

typedef enum {
    // Punctuations tokens
    T_COMMA,
    T_LPAREN,
    T_RPAREN,

    // Constants tokens
    T_STRING,
    T_SCALAR,

    T_DIRECTIVE,
    T_LABEL,
    T_REGISTER,
    T_IDENTIFIER,
    T_EOF,
    T_ERROR
} TokenType;

typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
    union {
        char* string;
        int32_t scalar;
    } value;
} Token;

typedef struct {
    Token* tokens;
    int capacity;
    int count;
} TokenList;

void initTokenList(TokenList* list);
void freeTokenList(TokenList* list);
void addToken(TokenList* list, Token token);

#endif //LMIPS_TOKENS
