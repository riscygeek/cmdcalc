#include <stdio.h>
#include "strint.h"
#include "parser.h"
#include "buf.h"

struct variable {
	const char* name;
	int value;
};

static Token* tokens = NULL;
static size_t tokenpos = 0;
static int parse_error = 0;
static int quite_error = 0;
static struct variable* vars = NULL;

static void set_var(const char* name, int value) {
	const size_t vars_len = buf_len(vars);
	name = strint(name);
	for (size_t i = 0; i < vars_len; ++i) {
		if (vars[i].name == name) { vars[i].value = value; return; }
	}
	buf_push(vars, ((struct variable){ name, value }));
}
static int* get_var(const char* name) {
	const size_t vars_len = buf_len(vars);
	name = strint(name);
	for (size_t i = 0; i < vars_len; ++i) {
		if (vars[i].name == name) return &vars[i].value;
	}
	return NULL;
}

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
	else if (is_token(TOKEN_NAME)) {
		const char* name = next_token().strVal;
		const int* value = get_var(name);

		if (!value) {
			printf("Variable %s not found!\n", name);
			quite_error = 1;
			return 0;
		}
		return *value;
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
		if (parse_error || quite_error) return 0;
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
	if (parse_error || quite_error) return 0;
	while (is_token(TOKEN_STAR) || is_token(TOKEN_SLASH)) {
		const enum TokenType op = tokens[tokenpos].type;
		next_token();
		int b = unary();
		if (parse_error || quite_error) return 0;

		switch (op) {
		case TOKEN_STAR: a *= b; break;
		case TOKEN_SLASH: a /= b; break;
		default: parse_error = 1; break;
		}
	}
	return a;
}
static int expression(void) {
	if (parse_error || quite_error) return 0;
	int a = term();
	while (is_token(TOKEN_PLUS) || is_token(TOKEN_MINUS)) {
		const enum TokenType op = tokens[tokenpos].type;
		next_token();
		int b = term();
		if (parse_error || quite_error) return 0;
		switch (op) {
		case TOKEN_PLUS: a += b; break;
		case TOKEN_MINUS: a -= b; break;
		default: parse_error = 1; break;
		}
	}
	return a;
}
static void statement(void) {
	if (is_token(TOKEN_NAME)) {
		if (tokens[tokenpos + 1].type == TOKEN_EQUAL) {
			const char* name = tokens[tokenpos].strVal;
			tokenpos += 2;

			int n = expression(); if (parse_error) {
				fputs("expected expression got ", stdout);
				print_token(tokens[tokenpos]);
			}
			else if (!quite_error) set_var(name, n);
		}
		else goto expr;
	}
	else {
	expr:;
		int n = expression();
		if (parse_error) {
			fputs("expected expression got ", stdout);
			print_token(tokens[tokenpos]);
		}
		else if (!quite_error) printf("%d\n", n);
	}
}

void parse(Token* tk) {
	tokens = tk;
	tokenpos = 0;
	parse_error = 0;
	quite_error = 0;
	statement();
}
