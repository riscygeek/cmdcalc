#ifndef FILE_TOKEN_H
#define FILE_TOKEN_H
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

enum TokenType {
	TK_DUMMY,
	TK_INTEGER, TK_FLOAT, TK_NAME,
	
	TK_PLUS, TK_MINUS, TK_STAR, TK_STARSTAR, TK_SLASH,
	TK_LPAREN, TK_RPAREN,
	TK_COMMA,
	TK_EQUALS,
	TK_EOF,
	
	NUM_TOKENS,
};

typedef struct {
	size_t begin, end;
} tokenpos_t;

typedef uintmax_t tkuint_t;
typedef intmax_t  tkint_t;
typedef double tkfloat_t;

typedef struct {
	enum TokenType type;
	tokenpos_t pos;
	union {
		tkuint_t    uVal;
		tkint_t     iVal;
		tkfloat_t   fVal;
		const char* str;
	};
} Token;

void print_token(Token tk, FILE* file);
extern const char* token_type_names[NUM_TOKENS];

#endif /* FILE_TOKEN_H */
