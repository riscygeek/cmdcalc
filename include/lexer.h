#ifndef FILE_LEXER_H
#define FILE_LEXER_H
#include <stdbool.h>
#include "token.h"

void lexer_init(void);
void lexer_free(void);
Token lexer_next(void);
Token lexer_peek(void);
void lexer_skip(void);

bool lexer_eof(void);
bool lexer_matches(enum TokenType tk);
bool lexer_match(enum TokenType tk);
Token lexer_expect(enum TokenType tk);

#endif /* FILE_LEXER_H */
