#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "evaluater.h"
#include "parser.h"
#include "lexer.h"
#include "input.h"


int main(const int argc, const char* argv[]) {
	evaluation_context_t* ctx = create_evaluation_context();
	char* input;
	while (true) {
		rl_bind_key('\t', rl_complete);
		input = readline("> ");
		if (!input || strcmp(input, "exit") == 0) break;
		add_history(input);
		
		input_init_str(input);
		lexer_init();
		
		Expression* expr = parse_expr();
		const tkfloat_t value = evaluate(ctx, expr);
		printf("%f\n", value);
		
		free_expr(expr);
		lexer_free();
		input_free();
		
		free(input);
	}
	return 0;
}
