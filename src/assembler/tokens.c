#include <stdlib.h>
#include "tokens.h"

void resetTokenList(TokenList* list) {
    list->capacity = 0;
    list->count = 0;
    list->tokens = NULL;
}

void initTokenList(TokenList* list) {
    resetTokenList(list);
    list->capacity = 50;
    list->tokens = realloc(list->tokens, list->capacity * sizeof(Token));
}

void freeTokenList(TokenList* list) {
    list->tokens = realloc(list->tokens, 0);
    resetTokenList(list);
}

void addToken(TokenList* list, Token token) {
    if (list->count == list->capacity) {
        list->capacity *= 2;
        list->tokens = realloc(list->tokens, list->capacity * sizeof(Token));
    }

    list->tokens[list->count++] = token;
}
