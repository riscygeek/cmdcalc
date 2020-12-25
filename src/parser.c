#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "buf.h"

#define alloc(t) ((t*)malloc(sizeof(t)))

static Expression* expr_addition(void);
static Expression* expr_prim(void) {
	Expression* expr = alloc(Expression);
	if (lexer_matches(TK_INTEGER)) {
		const Token tk = lexer_next();
		expr->type = EXPR_INTEGER;
		expr->pos = tk.pos;
		expr->uVal = tk.uVal;
		return expr;
	}
	else if (lexer_matches(TK_FLOAT)) {
		const Token tk = lexer_next();
		expr->type = EXPR_FLOAT;
		expr->pos = tk.pos;
		expr->fVal = tk.fVal;
		return expr;
	}
	else if (lexer_matches(TK_NAME)) {
		const Token tk = lexer_next();
		if (lexer_match(TK_LPAREN)) {
			expr->type = EXPR_FCALL;
			expr->pos.begin = tk.pos.begin;
			expr->fcall.name = tk.str;
			expr->fcall.args = NULL;
			
			if (!lexer_matches(TK_RPAREN)) {
				do { buf_push(expr->fcall.args, expr_addition()); }
				while (!lexer_match(TK_COMMA));
			}
			
			expr->pos.end = lexer_expect(TK_RPAREN).pos.end;
		}
		else if (lexer_match(TK_EQUALS)) {
			expr->type = EXPR_ASSIGN;
			expr->pos.begin = tk.pos.begin;
			expr->assign.name = tk.str;
			expr->assign.expr = expr_addition();
			expr->pos.end = expr->assign.expr->pos.end;
		}
		else {
			expr->type = EXPR_NAME;
			expr->pos = tk.pos;
			expr->str = tk.str;
		}
		return expr;
	}
	else if (lexer_matches(TK_LPAREN)) {
		expr->type = EXPR_PAREN;
		expr->pos.begin = lexer_next().pos.begin;
		expr->expr = parse_expr();
		expr->pos.end = lexer_expect(TK_RPAREN).pos.end;
		return expr;
	}
	else {
		// TODO: error()
		printf("expected expression got ");
		print_token(lexer_peek(), stderr);
		exit(EXIT_FAILURE);
	}
}
static Expression* expr_unary(void) {
	if (lexer_matches(TK_PLUS) || lexer_matches(TK_MINUS)) {
		Expression* expr = alloc(Expression);
		expr->type = EXPR_UNARY;
		expr->unary.op = lexer_next();
		expr->pos.begin = expr->unary.op.pos.begin;
		expr->unary.expr = expr_unary();
		expr->pos.end = expr->unary.expr->pos.end;
		return expr;
	}
	else return expr_prim();
}
static Expression* expr_exponent(void) {
	Expression* left = expr_unary();
	while (lexer_matches(TK_STARSTAR)) {
		Expression* expr = alloc(Expression);
		expr->type = EXPR_BINARY;
		expr->binary.left = left;
		expr->binary.op = lexer_next();
		expr->pos.begin = left->pos.begin;
		expr->binary.right = expr_unary();
		expr->pos.end = expr->binary.right->pos.end;
		left = expr;
	}
	return left;
}
static Expression* expr_factor(void) {
	Expression* left = expr_exponent();
	while (lexer_matches(TK_STAR) || lexer_matches(TK_SLASH)) {
		Expression* expr = alloc(Expression);
		expr->type = EXPR_BINARY;
		expr->binary.left = left;
		expr->binary.op = lexer_next();
		expr->pos.begin = left->pos.begin;
		expr->binary.right = expr_exponent();
		expr->pos.end = expr->binary.right->pos.end;
		left = expr;
	}
	return left;
}

static Expression* expr_addition(void) {
	Expression* left = expr_factor();
	while (lexer_matches(TK_PLUS) || lexer_matches(TK_MINUS)) {
		Expression* expr = alloc(Expression);
		expr->type = EXPR_BINARY;
		expr->binary.left = left;
		expr->binary.op = lexer_next();
		expr->pos.begin = left->pos.begin;
		expr->binary.right = expr_factor();
		expr->pos.end = expr->binary.right->pos.end;
		left = expr;
	}
	return left;
}
Expression* parse_expr(void) { return expr_addition(); }


void print_expr(const Expression* expr, FILE* file) {
	switch (expr->type) {
	case EXPR_INTEGER:  fprintf(file, "%ju", expr->uVal); break;
	case EXPR_FLOAT:    fprintf(file, "%f", expr->fVal); break;
	case EXPR_NAME:     fputs(expr->str, file); break;
	case EXPR_PAREN:
		fputc('(', file);
		print_expr(expr->expr, file);
		fputc(')', file);
		break;
	case EXPR_UNARY:
		print_token(expr->unary.op, file);
		print_expr(expr->unary.expr, file);
		break;
	case EXPR_BINARY:
		print_expr(expr->binary.left, file);
		fputc(' ', file);
		print_token(expr->binary.op, file);
		fputc(' ', file);
		print_expr(expr->binary.right, file);
		break;
	case EXPR_FCALL:
		fputs(expr->fcall.name, file);
		fputc('(', file);
		if (buf_len(expr->fcall.args)) {
			print_expr(expr->fcall.args[0], file);
			for (size_t i = 1; i < buf_len(expr->fcall.args); ++i) {
				fputs(", ", file);
				print_expr(expr->fcall.args[i], file);
			}
		}
		fputc(')', file);
		break;
	case EXPR_ASSIGN:
		fprintf(file, "%s = ", expr->assign.name);
		print_expr(expr->assign.expr, file);
		break;
	}
}

void free_expr(Expression* expr) {
	switch (expr->type) {
	case EXPR_PAREN:    free_expr(expr->expr); break;
	case EXPR_UNARY:    free_expr(expr->unary.expr); break;
	case EXPR_ASSIGN:   free_expr(expr->assign.expr); break;
	case EXPR_BINARY:
		free_expr(expr->binary.left);
		free_expr(expr->binary.right);
		break;
	case EXPR_FCALL:
		for (size_t i = 0; i < buf_len(expr->fcall.args); ++i)
			free_expr(expr->fcall.args[i]);
		buf_free(expr->fcall.args);
		break;
	default: break;
	}
	free(expr);
}