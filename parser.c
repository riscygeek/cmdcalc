#include <string.h>
#include <math.h>
#include "console.h"
#include "strint.h"
#include "parser.h"
#include "buf.h"

#define VERSION 4
#define MAX_FPARAMS 4

struct variable {
	const char* name;
	int value;
};
struct function {
	const char* name;
	int paramcount;
	int builtin;
	union {
		int(*callback)();
		Token* tokens;
	};
	const char* paramnames[MAX_FPARAMS];
};
struct stack_frame {
	struct variable* vars;
	const struct function* func;
	Token* saved_tokens;
	size_t saved_tokenpos;
};

static Token* tokens = NULL;
static size_t tokenpos = 0;
static int quite_error = 0;
static struct function* funcs = NULL;
static struct stack_frame* stack = NULL;

static void set_var(const char* name, int value) {
	struct variable* vars = buf_last(stack)->vars;
	const size_t vars_len = buf_len(vars);
	name = strint(name);
	for (size_t i = 0; i < vars_len; ++i) {
		if (vars[i].name == name) { vars[i].value = value; return; }
	}
	buf_push(vars, ((struct variable){ name, value }));
	buf_last(stack)->vars = vars;
}
static int* get_var(const char* name) {
	struct variable* vars = buf_last(stack)->vars;
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

static struct function* get_func(const char* name) {
	const size_t funcs_len = buf_len(funcs);
	name = strint(name);
	for (size_t i = 0; i < funcs_len; ++i) {
		if (funcs[i].name == name) return &funcs[i];
	}
	return NULL;
}
void init_funcs(void) {
#define add(name, pc, cb) buf_push(funcs, ((struct function){ strint(name), pc, 1, cb }))
	add("sqr", 1, func_sqr);
	add("sqrt", 1, func_sqrt);
	add("version", 0, func_version);
	add("add", 2, func_add);
#undef add
	buf_push(stack, ((struct stack_frame){ NULL, NULL }));
}
static int expression();
static void new_frame(const struct function* f, Token* saved1, size_t saved2) {
	buf_push(stack, ((struct stack_frame){ NULL, f, saved1, saved2 }));
}
static void pop_frame() {
	buf_free(buf_last(stack)->vars);
	buf_pop(stack);
}
static int call(const char* name, int params[MAX_FPARAMS], int nParams) {
	const struct function* f = get_func(name);
	if (!f) {
		outform("Function %s not found!\n", name);
		quite_error = 1;
		return 0;
	}
	else if (nParams != f->paramcount) {
		outform("Function %s expects %d parameters\n", name, f->paramcount);
		quite_error = 1;
		return 0;
	}
	else if (f->paramcount < 0 || f->paramcount > MAX_FPARAMS) {
		outform("Function %s is invalid\n", name);
		quite_error = 1;
		return 0;
	}
	else {
		if (f->builtin) {
			switch (nParams) {
			case 0: return f->callback();
			case 1: return f->callback(params[0]);
			case 2: return f->callback(params[0], params[1]);
			case 3: return f->callback(params[0], params[1], params[2]);
			case 4: return f->callback(params[0], params[1], params[2], params[3]);
			default:
				outform("Some Error occured on calling %s!\n", name);
				quite_error = 1;
				return 0;
			}
		}
		else {
			if (nParams != f->paramcount) {
				outform("Function %s expects %d params!\n", name, f->paramcount);
				quite_error = 1;
				return 0;
			}
			new_frame(f, tokens, tokenpos);
			tokens = f->tokens;
			tokenpos = 0;

			for (int i = 0; i < nParams; ++i) {
				set_var(f->paramnames[i], params[i]);
			}

			int r = expression();

			pop_frame();
			return r;
		}
	}
}
static void defun(const char* name, int paramcount, size_t tokenpos, const char* paramnames[MAX_FPARAMS]) {
	if (get_func(name)) {
		struct function* f = get_func(name);
		if (f->builtin) {
			outform("Cannot redefine builtin function %s!\n", name);
			return;
		}
		f->paramcount = paramcount;
		f->tokens = &tokens[tokenpos];
		for (size_t i = 0; i < MAX_FPARAMS; ++i) f->paramnames[i] = paramnames[i];
	}
	else {
		struct function f = { name, paramcount, 0, .tokens = &tokens[tokenpos] };
		for (size_t i = 0; i < MAX_FPARAMS; ++i) f.paramnames[i] = paramnames[i];
		buf_push(funcs, f);
	}
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
			int params[MAX_FPARAMS];
			if (match_token(TOKEN_RPAREN))
				return call(name, params, 0);
			params[0] = expression();
			size_t i = 1;
			while (i < MAX_FPARAMS && match_token(TOKEN_COMMA)) {
				params[i] = expression();
				++i;
			}
			if (is_token(TOKEN_COMMA)) {
				outform("Passed more than %d arguments!\n", MAX_FPARAMS);
				quite_error = 1;
				return 0;
			}
			if (!match_token(TOKEN_RPAREN)) {
				outstr("expected ')' got ");
				print_token(tokens[tokenpos]);
				quite_error = 1;
				return 0;
			}
			return call(name, params, i);
		}
		const int* value = get_var(name);

		if (!value) {
			outform("Variable %s not found!\n", name);
			quite_error = 1;
			return 0;
		}
		return *value;
	}
	else if (match_token(TOKEN_LPAREN)) {
		int a = expression();
		if (match_token(TOKEN_RPAREN)) return a;
		else {
			outstr("expected ')' got ");
			print_token(tokens[tokenpos++]);
			quite_error = 1;
			return 0;
		}
	}
	else {
		outstr("expected number, name or '(' got ");
		print_token(tokens[tokenpos++]);
		quite_error = 1;
		return 0;
	}
}
static int unary(void) {
	if (is_token(TOKEN_PLUS) || is_token(TOKEN_MINUS)) {
		const enum TokenType op = tokens[tokenpos].type;
		next_token();
		int b = unary();
		if (quite_error) return 0;
		switch (op) {
		case TOKEN_PLUS: return b;
		case TOKEN_MINUS: return -b;
		default:
			outstr("expected '+' or '-' got ");
			print_token(tokens[tokenpos]);
			quite_error = 1;
			break;
		}
	}
	else return factor();
}
static int term(void) {
	int a = unary();
	if (quite_error) return 0;
	while (is_token(TOKEN_STAR) || is_token(TOKEN_SLASH)) {
		const enum TokenType op = tokens[tokenpos].type;
		next_token();
		int b = unary();
		if (quite_error) return 0;

		switch (op) {
		case TOKEN_STAR: a *= b; break;
		case TOKEN_SLASH: a /= b; break;
		default:
			outstr("expected '*' or '/' got ");
			print_token(tokens[tokenpos]);
			quite_error = 1;
			break;
		}
	}
	return a;
}
static int expression(void) {
	if (quite_error) return 0;
	int a = term();
	if (quite_error) return 0;
	while (is_token(TOKEN_PLUS) || is_token(TOKEN_MINUS)) {
		const enum TokenType op = tokens[tokenpos].type;
		next_token();
		int b = term();
		if (quite_error) return 0;
		switch (op) {
		case TOKEN_PLUS: a += b; break;
		case TOKEN_MINUS: a -= b; break;
		default:
			outstr("expected '+' or '-' got ");
			print_token(tokens[tokenpos]);
			quite_error = 1;
			break;
		}
	}
	return a;
}
static void statement(void) {
	if (is_token(TOKEN_NAME)) {
		if (strcmp(tokens[tokenpos].strVal, "funcs") == 0) {
			const size_t funcs_len = buf_len(funcs);
			next_token();
			outstr("Functions:\n");
			for (size_t i = 0; i < funcs_len; ++i) {
				outform("%s\n", funcs[i].name);
			}
		}
		else if (strcmp(tokens[tokenpos].strVal, "vars") == 0) {
			const struct variable* vars = buf_last(stack)->vars;
			const size_t vars_len = buf_len(vars);
			next_token();
			outstr("Variables:\n");
			for (size_t i = 0; i < vars_len; ++i) {
				outform("%s\n", vars[i].name);
			}
		}
		else if (tokens[tokenpos + 1].type == TOKEN_EQUAL) {
			const char* name = tokens[tokenpos].strVal;
			tokenpos += 2;

			int n = expression();
			if (!quite_error) set_var(name, n);
		}
		else goto expr;
	}
	// fun sub(a, b) = a - b
	else if (match_token(TOKEN_FUN)) {
		const char* paramnames[MAX_FPARAMS] = { NULL };
		int paramcount = 0;
		if (!is_token(TOKEN_NAME)) {
			outstr("expected name got ");
			print_token(tokens[tokenpos++]);
			quite_error = 1;
			return;
		}
		const char* name = next_token().strVal;
		if (!match_token(TOKEN_LPAREN)) {
			outstr("expected '(' got ");
			print_token(tokens[tokenpos++]);
			quite_error = 1;
		}
		else if (match_token(TOKEN_RPAREN)) {
			if (!match_token(TOKEN_EQUAL)) {
				outstr("expected '=' got ");
				print_token(tokens[tokenpos++]);
				quite_error = 1;
			}
			else defun(name, 0, tokenpos, (const char**)paramnames);
		}
		else {
			if (!is_token(TOKEN_NAME)) {
				outstr("expected name got ");
				print_token(tokens[tokenpos++]);
				quite_error = 1;
				return;
			}
			paramnames[0] = next_token().strVal;
			paramcount = 1;
			while (paramcount < MAX_FPARAMS && match_token(TOKEN_COMMA)) {
				if (!is_token(TOKEN_NAME)) {
					outstr("expected name got ");
					print_token(tokens[tokenpos++]);
					quite_error = 1;
					return;
				}
				paramnames[paramcount++] = next_token().strVal;
			}
			if (!match_token(TOKEN_RPAREN)) {
				outstr("expected ')' got ");
				print_token(tokens[tokenpos++]);
				quite_error = 1;
				return;
			}
			if (!match_token(TOKEN_EQUAL)) {
				outstr("expected '=' got ");
				print_token(tokens[tokenpos++]);
				quite_error = 1;
				return;
			}

			defun(name, paramcount, tokenpos, paramnames);
		}
		
	}
	else {
	expr:;
		int n = expression();
		if (!quite_error) outform("%d\n", n);
	}
}

void parse(Token* tk) {
	tokens = tk;
	tokenpos = 0;
	quite_error = 0;
	statement();
}
