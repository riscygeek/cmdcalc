#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "strint.h"
#include "lexer.h"
#include "buf.h"

static const char* textpos = NULL;
static const char* textend = NULL;
static const char* tokenNames[] = {
	"unknown", "number",
	"operator", "operator", "operator", "operator",
	"parenthesis", "parenthesis",

	"end of file",
};

void print_token(const Token tk) {
	printf("Token{ .type=%s, .text=\"%s\"", tokenNames[(size_t)tk.type], tk.text);
	switch (tk.type) {
	case TOKEN_NUMBER: printf(", .value=%d", tk.intVal); break;
	case TOKEN_PLUS:
	case TOKEN_MINUS:
	case TOKEN_STAR:
	case TOKEN_SLASH:
	case TOKEN_LPAREN:
	case TOKEN_RPAREN:
		printf(", .value='%c'", tk.intVal);
		break;
	}
	puts(" }");
}
void print_tokens(const Token* tokens) {
	const size_t len = buf_len(tokens);
	for (size_t i = 0; i < len; ++i) print_token(tokens[i]);
}
static Token next_token(void) {
begin:;
	if (textpos == textend) return ((Token) { TOKEN_EOF, "", 0 });
	else if (isspace(*textpos)) {
		while (isspace(*++textpos));
		goto begin;
	}
	else if (isdigit(*textpos)) {
		const char* start = textpos;
		int num = 0;
		while (isdigit(*textpos)) num = num * 10 + (*textpos++ - '0');
		return ((Token) { TOKEN_NUMBER, strrint(start, textpos), num });
	}
	else if (*textpos == '+')
		return ((Token) { TOKEN_PLUS, strnint(textpos, 1), * textpos++ });
	else if (*textpos == '-')
		return ((Token) { TOKEN_MINUS, strnint(textpos, 1), * textpos++ });
	else if (*textpos == '*')
		return ((Token) { TOKEN_STAR, strnint(textpos, 1), * textpos++ });
	else if (*textpos == '/')
		return ((Token) { TOKEN_SLASH, strnint(textpos, 1), * textpos++ });
	else if (*textpos == '(')
		return ((Token) { TOKEN_LPAREN, strnint(textpos, 1), * textpos++ });
	else if (*textpos == ')')
		return ((Token) { TOKEN_RPAREN, strnint(textpos, 1), * textpos++ });

	else return  ((Token) { TOKEN_UNKNOWN, strnint(textpos, 1), *textpos++ });
}

Token* lex(const char* str) {
	Token* tokens = NULL;
	textpos = str;
	textend = str + strlen(str);
	while (1) {
		Token tk = next_token();
		buf_push(tokens, tk);
		if (tk.type == TOKEN_EOF) break;
	}
	return tokens;
}