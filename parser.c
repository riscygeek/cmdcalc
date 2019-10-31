#include <stdio.h>
#include "parser.h"
#include "buf.h"

static Token* tokens = NULL;
static size_t tokenpos = 0;
static int parse_error = 0;

inline static int is_token(enum TokenType type) {
	return tokens[tokenpos].type == type;
}
inline static Token next_token(void) {
	return tokenpos < buf_len(tokens) ? tokens[tokenpos++] : *buf_last(tokens);
}
inline static int match_token(enum TokenType type) {
	if (is_token(type)) return next_token(), 1;
	else return 0;
}

static int expression(void);
static int factor(void) {
	if (is_token(TOKEN_NUMBER)) {
		return next_token().intVal;
	}
	else if (match_token(TOKEN_LPAREN)) {
		int a = expression();
		if (is_token(TOKEN_RPAREN)) return next_token(), a;
		else return parse_error = 1, 0;
	}
	else return parse_error = 1, 0;
}
static int unary(void) {
	if (is_token(TOKEN_PLUS) || is_token(TOKEN_MINUS)) {
		const enum TokenType op = tokens[tokenpos].type;
		next_token();
		int b = unary();
		switch (op) {
		case TOKEN_PLUS: return b;
		case TOKEN_MINUS: return -b;
		default: parse_error = 1; return 0;
		}
	}
	else return factor();
}
static int term(void) {
	int a = unary();
	while (is_token(TOKEN_STAR) || is_token(TOKEN_SLASH)) {
		const enum TokenType op = tokens[tokenpos].type;
		next_token();
		int b = unary();

		switch (op) {
		case TOKEN_STAR: a *= b; break;
		case TOKEN_SLASH: a /= b; break;
		default: parse_error = 1; break;
		}
	}
	return a;
}
static int expression(void) {
	int a = term();
	while (is_token(TOKEN_PLUS) || is_token(TOKEN_MINUS)) {
		const enum TokenType op = tokens[tokenpos].type;
		next_token();
		int b = term();
		switch (op) {
		case TOKEN_PLUS: a += b; break;
		case TOKEN_MINUS: a -= b; break;
		default: parse_error = 1; break;
		}
	}
	return a;
}

void parse(Token* tk) {
	tokens = tk;
	tokenpos = 0;
	parse_error = 0;
	int n = expression();
	if (parse_error) printf("parse_error!\n");
	else printf("%d\n", n);
}
