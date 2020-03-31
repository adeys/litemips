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

char peek(Lexer* lexer);
char advance(Lexer* lexer);
bool isAtEnd(Lexer* lexer);
void skipWhitespace(Lexer*);
Token getNextToken(Lexer*);
Token makeToken(TokenType type, Lexer* lexer);

#endif //LMIPS_LEXER
