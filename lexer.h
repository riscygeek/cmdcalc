#ifndef __LEXER_H__
#define __LEXER_H__
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum TokenType {
	TOKEN_UNKNOWN, TOKEN_NUMBER,
	TOKEN_PLUS, TOKEN_MINUS, TOKEN_STAR, TOKEN_SLASH,
	TOKEN_LPAREN, TOKEN_RPAREN,

	TOKEN_EOF,
};
	
typedef struct Token {
	enum TokenType type;
	const char* text;
	union {
		int intVal;
		const char* strVal;
	};
} Token;

void print_token(const Token);
void print_tokens(const Token*);
Token* lex(const char*);

#ifdef __cplusplus
}
#endif

#endif /* __LEXER_H__ */