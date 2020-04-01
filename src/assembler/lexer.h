#ifndef LMIPS_LEXER
#define LMIPS_LEXER

#include "common.h"
#include "tokens.h"

typedef enum {
    STATE_OK,
    STATE_ERROR
} LexerState;

typedef struct {
    const char* source;
    const char* start;
    const char* current;
    TokenList tokens;
    int line;
    LexerState state;
} Lexer;

void initLexer(Lexer* lexer);
void tokenize(Lexer* lexer, const char* program);
void freeLexer(Lexer* lexer);

#endif //LMIPS_LEXER
