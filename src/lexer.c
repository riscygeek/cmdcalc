#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "strint.h"
#include "input.h"
#include "lexer.h"
#include "buf.h"

static Token peekd;

void lexer_init(void) {
	peekd.type = TK_DUMMY;
}
void lexer_free(void) {}

static Token lexer_impl(void);
Token lexer_next(void) {
	if (peekd.type) {
		const Token tk = peekd;
		peekd.type = TK_DUMMY;
		return tk;
	}
	else return lexer_impl();
}
Token lexer_peek(void) {
	return peekd.type ? peekd : (peekd = lexer_impl());
}
void lexer_skip(void) {
	if (peekd.type) peekd.type = TK_DUMMY;
	else lexer_impl();
}
bool lexer_eof(void) {
	return peekd.type == TK_EOF || (!peekd.type && input_eof());
}
bool lexer_matches(const enum TokenType type) {
	return lexer_peek().type == type;
}
bool lexer_match(const enum TokenType type) {
	if (lexer_matches(type)) return lexer_skip(), true;
	else return false;
}
Token lexer_expect(const enum TokenType type) {
	const Token tk = lexer_next();
	if (tk.type == type) return tk;
	else {
		// TODO: error handling
		printf("invalid token at %zu\n", tk.pos.begin);
		exit(EXIT_FAILURE);
	}
}
static bool isname(const char ch) {
	return isalnum(ch) || (ch == '_');
}
static Token lexer_impl(void) {
	while (isspace(input_peek())) input_skip();
	const size_t start = input_pos();
	char ch = input_peek();
	if (isdigit(ch)) {
		tkuint_t ipart = 0;
		while (isdigit(input_peek())) ipart = ipart * 10 + (input_next() - '0');
		if (input_peek() == '.') {
			tkfloat_t fpart = 0.0;
			input_skip();
			int exp = 0;
			while (isdigit(input_peek())) fpart += ((input_next() - '0') * pow(10.0, --exp));
			return (Token){ TK_FLOAT, { start, input_pos() }, .fVal = (ipart + fpart) };
		}
		else return (Token){ TK_INTEGER, { start, input_pos() }, .uVal = ipart };
	}
	else if (isalpha(ch) || (ch == '_')) {
		char* buf = NULL;
		while (isname(input_peek()))
			buf_push(buf, input_next());
		buf_push(buf, '\0');
		const char* name = strint(buf);
		buf_free(buf);
		return (Token){ TK_NAME, { start, input_pos() }, .str = name };
	}
	else {
		input_skip();
		switch (ch) {
		case '+':   return (Token){ TK_PLUS, {start, start} };
		case '-':   return (Token){ TK_MINUS, {start, start} };
		case '*':
			if (input_peek() == '*')
				return input_skip(), (Token){ TK_STARSTAR, {start,start} };
			else return (Token){ TK_STAR, {start, start} };
		case '/':   return (Token){ TK_SLASH, {start, start} };
		case '(':   return (Token){ TK_LPAREN, {start, start} };
		case ')':   return (Token){ TK_RPAREN, {start, start} };
		case ',':   return (Token){ TK_COMMA, {start, start} };
		case '=':   return (Token){ TK_EQUALS, {start, start} };
		case '\0':  return (Token){ TK_EOF, {start, start} };
		default:
			// TODO: error()
			printf("illegal input '%c' at %zu\n", ch, start);
			exit(1);
		}
	}
}