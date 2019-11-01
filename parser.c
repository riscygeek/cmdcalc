#include <string.h>
#include <stdio.h>
#include <math.h>
#include "strint.h"
#include "parser.h"
#include "buf.h"

#define VERSION 3

#define MAX_FPARAMS 2
typedef int(*func0_t)();
typedef int(*func1_t)(int);
typedef int(*func2_t)(int, int);

struct variable {
	const char* name;
	int value;
};
struct function {
	const char* name;
	int paramcount;
	void* callback;
};

static Token* tokens = NULL;
static size_t tokenpos = 0;
static int parse_error = 0;
static int quite_error = 0;
static struct variable* vars = NULL;
static struct function* funcs = NULL;

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

static int func_sqr(int x) { return x * x; }
static int func_sqrt(int x) { return (int)sqrtf(x); }
static int func_version() { return VERSION; }
static int func_add(int a, int b) { return a + b; }

static const struct function* get_func(const char* name) {
	const size_t funcs_len = buf_len(funcs);
	name = strint(name);
	for (size_t i = 0; i < funcs_len; ++i) {
		if (funcs[i].name == name) return &funcs[i];
	}
	return NULL;
}
void init_funcs(void) {
#define add(name, pc, cb) buf_push(funcs, ((struct function){ strint(name), pc, cb }))
	add("sqr", 1, func_sqr);
	add("sqrt", 1, func_sqrt);
	add("version", 0, func_version);
	add("add", 2, func_add);
#undef add
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
		if (match_token(TOKEN_LPAREN)) { // a = sqr(42)
			const struct function* f = get_func(name);
			if (!f) {
				printf("Function %s not found!\n", name);
				quite_error = 1;
				return 0;
			}
			else if (f->paramcount < 0 || f->paramcount > MAX_FPARAMS) {
				printf("Function %s is invalid\n", name);
				quite_error = 1;
				return 0;
			}
			else if (match_token(TOKEN_RPAREN)) {
				if (f->paramcount == 0) return ((func0_t)f->callback)();
				else {
					printf("Function %s expects %d parameters\n", name, f->paramcount);
					quite_error = 1;
					return 0;
				}
			}
			int params[MAX_FPARAMS];
			params[0] = expression();
			size_t i = 1;
			while (i < MAX_FPARAMS && match_token(TOKEN_COMMA)) {
				params[i] = expression();
				++i;
			}
			if (is_token(TOKEN_COMMA)) {
				printf("Passed more than %d arguments!\n", MAX_FPARAMS);
				quite_error = 1;
				return 0;
			}
			if (!match_token(TOKEN_RPAREN)) {
				fputs("expected ')' got ", stdout);
				print_token(tokens[tokenpos]);
				quite_error = 1;
				return 0;
			}
			if (i != f->paramcount) {
				printf("Function %s expects %d parameters!\n", name, f->paramcount);
				quite_error = 1;
				return 0;
			}
			switch (i) {
			case 1: return ((func1_t)f->callback)(params[0]); break;
			case 2: return ((func2_t)f->callback)(params[0], params[1]); break;
			}
		}
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
		if (strcmp(tokens[tokenpos].strVal, "funcs") == 0) {
			const size_t funcs_len = buf_len(funcs);
			next_token();
			puts("Functions:");
			for (size_t i = 0; i < funcs_len; ++i) {
				puts(funcs[i].name);
			}
		}
		else if (strcmp(tokens[tokenpos].strVal, "vars") == 0) {
			const size_t vars_len = buf_len(vars);
			next_token();
			puts("Variables:");
			for (size_t i = 0; i < vars_len; ++i) {
				puts(vars[i].name);
			}
		}
		else if (tokens[tokenpos + 1].type == TOKEN_EQUAL) {
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
