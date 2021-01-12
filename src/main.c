#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "evaluater.h"
#include "strint.h"
#include "parser.h"
#include "lexer.h"
#include "input.h"
#include "error.h"
#include "buf.h"

static bool isname1(const char ch) {
	return isalpha(ch) || (ch == '_');
}
static bool isname(const char ch) {
	return isalnum(ch) || (ch == '_');
}
static const char* parse_name(const char* str, size_t* const i) {
	char* buf = NULL;
	if (!isname1(str[*i])) {
		fprintf(stderr, "%*s|: expected name\n", *i + 2, "");
		return NULL;
	}
	buf_push(buf, str[*i]);
	for (++*i; isname(str[*i]); ++*i) buf_push(buf, str[*i]);
	buf_push(buf, '\0');
	const char* name = strint(buf);
	buf_free(buf);
	return name;
}
static void signal_handler(int signo) {
	if (signo == SIGINT) {
		putchar('\n');
		rl_on_new_line();
		rl_replace_line("", 0);
		rl_redisplay();
	}
}
int main(const int argc, const char* argv[]) {
	evaluation_context_t* ctx = create_evaluation_context();
	evaluation_context_add_builtins(ctx);
	char* input;
	signal(SIGINT, signal_handler);
	rl_bind_key('\t', rl_complete);
	while (true) {
		input = readline("> ");
		if (!input || strcmp(input, "exit") == 0) break;
		else if (!input[0]) continue;
		add_history(input);
		
		errored = false;
		
		input_init_str(input);
		lexer_init();
		
		if (strcmp(input, "funcs") == 0) {
			for (size_t i = 0; i < buf_len(ctx->funcs); ++i)
				puts(ctx->funcs[i].name);
			continue;
		}
		else if (strcmp(input, "vars") == 0) {
			for (size_t i = 0; i < buf_len(ctx->vars); ++i)
				puts(ctx->vars[i].name);
			continue;
		}
		else if (memcmp(input, "func", 4) == 0) {
#define skip_space() while (isspace(input[i])) ++i
			const char** paramnames = NULL;
			size_t i;
			if (!isspace(input[4])) {
				fputs("      |: expected white-space", stderr);
				continue;
			}
			for (i = 5; isspace(input[i]); ++i);
			if (!isname1(input[i])) {
				fprintf(stderr, "%*s^: expected name\n", i + 2, "");
				continue;
			}
			const char* name = parse_name(input, &i);
			skip_space();
			if (input[i] != '(') {
				fprintf(stderr, "%*s^: expected '('\n", i + 2, "");
				continue;
			}
			++i;
			skip_space();
			if (input[i] != ')') {
				do {
					skip_space();
					const char* pname = parse_name(input, &i);
					if (!pname) goto skip;
					buf_push(paramnames, pname);
					skip_space();
				} while (input[i] == ',' && ++i);
			}
			if (input[i] != ')') {
				fprintf(stderr, "%*s^: expected ')'\n", i + 2, "");
				skip: continue;
			}
			++i;
			skip_space();
			if (input[i] != '=') {
				fprintf(stderr, "%*s^: expected '='\n", i + 2, "");
				continue;
			}
			
			input_init_str(&input[i + 1]);
			lexer_init();
			Expression* body = parse_expr();
			if (errored) continue;
			lexer_free();
			input_free();
			
			evaluation_context_add_userfunc(ctx, name, paramnames, body);
			
			fflush(stderr);
			free(input);
			continue;
#undef skip_space
		}
		
		Expression* expr = parse_expr();
		if (!errored || !expr) {
			value_t* value = evaluate(ctx, expr);
			if (!errored && value->type != VALUE_EMPTY) {
				print_value(value, stdout);
				putchar('\n');
			}
			free_value(value);
		}
		
		free_expr(expr);
		lexer_free();
		input_free();
		
		free(input);
	}
	return 0;
}
