#include <stdio.h>
#include "cmdcalc.h"
#include "error.h"
#include "input.h"
#include "lexer.h"
#include "parser.h"
#include "evaluater.h"

int main(int argc, char* argv[]) {
	const struct cmdline_args args = parse_args(argc, argv);
	if (args.exitcode != -1) return args.exitcode;
	else if (args.filename) {
		puts("Executing Files is currently not supported!");
		return 1;
	}
	else if (args.expression) {
		input_init_str(args.expression);
		lexer_init();
		
		Expression* expr = parse_expr();
		if (errored) return lexer_free(), input_free(), 1;
		evaluation_context_t* ctx = create_evaluation_context();
		evaluation_context_add_builtins(ctx);
		
		value_t* value = evaluate(ctx, expr);
		free_evaluation_context(ctx);
		free_expr(expr);
		lexer_free();
		input_free();
		if (errored) return 1;
		
		print_value(value, stdout);
		free_value(value);
		putchar('\n');
		return 0;
	}
	else return run_shell();
}
