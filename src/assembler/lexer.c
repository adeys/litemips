#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lexer.h"

Token getScalar(Lexer* lexer, char c);
Token getIdentifier(Lexer* lexer);
Token getDirective(Lexer* lexer);
Token getRegister(Lexer* lexer);
Token getString(Lexer* lexer);
void reportError(const Token* token);

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

    return makeToken(type, lexer);
}

Token getDirective(Lexer* lexer) {
    while (isAlpha(peek(lexer))) {
        advance(lexer);
    }

    return makeToken(T_DIRECTIVE, lexer);
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