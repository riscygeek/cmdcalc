#include <stdlib.h>
#include "lexer.h"
#include "parser.h"
#include "error.h"
#include "buf.h"

#define alloc(t) ((t*)calloc(1, sizeof(t)))

static Expression* expr_addition(void);
static Expression* expr_conditional(void);
static Expression* parse_expr1(void);
static Expression* expr_prim(void) {
	Expression* expr = alloc(Expression);
	if (!expr || lexer_matches(TK_ERROR)) return NULL;
	else if (lexer_matches(TK_INTEGER)) {
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
	else if (lexer_matches(TK_STRING)) {
		const Token tk = lexer_next();
		expr->type = EXPR_STRING;
		expr->pos = tk.pos;
		expr->str = tk.str;
		return expr;
	}
	else if (lexer_matches(TK_LBRACK)) {
		expr->type = EXPR_ARRAY;
		expr->pos.begin = lexer_next().pos.begin;
		expr->comma = NULL;
		if (!lexer_matches(TK_RBRACK)) {
			do {
				Expression* elem = expr_conditional();
				if (!elem) return free_expr(expr), NULL;
				buf_push(expr->comma, elem);
			} while (lexer_match(TK_COMMA));
		}
		expr->pos.end = lexer_expect(TK_RBRACK).pos.end;
		return errored ? free_expr(expr), NULL : expr;
	}
	else if (lexer_matches(TK_NAME)) {
		const Token tk = lexer_next();
		if (lexer_match(TK_LPAREN)) {
			expr->type = EXPR_FCALL;
			expr->pos.begin = tk.pos.begin;
			expr->fcall.name = tk.str;
			expr->fcall.args = NULL;
			
			if (!lexer_matches(TK_RPAREN)) {
				do {
					Expression* e = expr_conditional();
					if (!e) {
						free_expr(expr);
						free_expr(e);
						return NULL;
					}
					else buf_push(expr->fcall.args, e);
				} while (lexer_match(TK_COMMA));
			}
			expr->pos.end = lexer_expect(TK_RPAREN).pos.end;
			if (errored) {
				free_expr(expr);
				return NULL;
			}
		}
		else if (lexer_match(TK_EQUALS)) {
			expr->type = EXPR_ASSIGN;
			expr->pos.begin = tk.pos.begin;
			expr->assign.name = tk.str;
			expr->assign.expr = expr_conditional();
			if (!expr->assign.expr) {
				free_expr(expr);
				return NULL;
			}
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
		expr->expr = parse_expr1();
		if (!expr->expr) {
			free_expr(expr);
			return NULL;
		}
		expr->pos.end = lexer_expect(TK_RPAREN).pos.end;
		if (errored) {
			free_expr(expr);
			return NULL;
		}
		else return expr;
	}
	else {
		error(lexer_peek().pos, "expected expression");
		free_expr(expr);
		return NULL;
	}
}
static Expression* expr_at(void) {
	Expression* base = expr_prim();
	if (!base) return NULL;
	while (lexer_match(TK_LBRACK)) {
		Expression* expr = alloc(Expression);
		expr->type = EXPR_AT;
		expr->pos.begin = base->pos.begin;
		expr->at.base = base;
		expr->at.index = parse_expr1();
		if (errored) return free_expr(expr), NULL;
		expr->pos.end = lexer_expect(TK_RBRACK).pos.end;
		if (errored) return free_expr(expr), NULL;
		base = expr;
	}
	return base;
}
static Expression* expr_unary(void) {
	if (lexer_matches(TK_PLUS) || lexer_matches(TK_MINUS)) {
		Expression* expr = alloc(Expression);
		expr->type = EXPR_UNARY;
		expr->unary.op = lexer_next();
		expr->pos.begin = expr->unary.op.pos.begin;
		expr->unary.expr = expr_unary();
		if (!expr->unary.expr) {
			free_expr(expr);
			return NULL;
		}
		expr->pos.end = expr->unary.expr->pos.end;
		return expr;
	}
	else return expr_at();
}
static Expression* expr_exponent(void) {
	Expression* left = expr_unary();
	if (!left) return NULL;
	while (lexer_matches(TK_STARSTAR)) {
		Expression* expr = alloc(Expression);
		expr->type = EXPR_BINARY;
		expr->binary.left = left;
		expr->binary.op = lexer_next();
		expr->pos.begin = left->pos.begin;
		expr->binary.right = expr_unary();
		if (!expr->binary.right) {
			free_expr(expr);
			free_expr(left);
			return NULL;
		}
		expr->pos.end = expr->binary.right->pos.end;
		left = expr;
	}
	return left;
}
static Expression* expr_factor(void) {
	Expression* left = expr_exponent();
	if (!left) return NULL;
	while (lexer_matches(TK_STAR) || lexer_matches(TK_SLASH)) {
		Expression* expr = alloc(Expression);
		expr->type = EXPR_BINARY;
		expr->binary.left = left;
		expr->binary.op = lexer_next();
		expr->pos.begin = left->pos.begin;
		expr->binary.right = expr_exponent();
		if (!expr->binary.right) {
			free_expr(left);
			free_expr(expr);
			return NULL;
		}
		expr->pos.end = expr->binary.right->pos.end;
		left = expr;
	}
	return left;
}

static Expression* expr_addition(void) {
	Expression* left = expr_factor();
	if (!left) return NULL;
	while (lexer_matches(TK_PLUS) || lexer_matches(TK_MINUS)) {
		Expression* expr = alloc(Expression);
		expr->type = EXPR_BINARY;
		expr->binary.left = left;
		expr->binary.op = lexer_next();
		expr->pos.begin = left->pos.begin;
		expr->binary.right = expr_factor();
		if (!expr->binary.right) {
			free_expr(left);
			free_expr(expr);
			return NULL;
		}
		expr->pos.end = expr->binary.right->pos.end;
		left = expr;
	}
	return left;
}
static Expression* expr_conditional(void) {
	Expression* cond = expr_addition();
	if (!cond) return NULL;
	if (!lexer_match(TK_QMARK)) return cond;
	Expression* expr = alloc(Expression);
	expr->type = EXPR_CONDITIONAL;
	expr->pos.begin = cond->pos.begin;
	expr->conditional.cond = cond;
	expr->conditional.true_case = expr_conditional();
	if (!expr->conditional.true_case) goto failed;
	lexer_expect(TK_COLON);
	if (errored) goto failed;
	expr->conditional.false_case = expr_conditional();
	if (!expr->conditional.false_case) goto failed;
	expr->pos.end = expr->conditional.false_case->pos.end;
	return expr;
failed:
	free_expr(expr);
	return NULL;
}
static Expression* expr_comma(void) {
	Expression* left = expr_conditional();
	if (!left) return NULL;
	if (lexer_matches(TK_COMMA)) {
		Expression* expr = alloc(Expression);
		expr->type = EXPR_COMMA;
		expr->pos.begin = left->pos.begin;
		expr->comma = NULL;
		buf_push(expr->comma, left);
		while (lexer_match(TK_COMMA)) {
			buf_push(expr->comma, expr_conditional());
			if (errored) {
				free_expr(expr);
				return NULL;
			}
		}
		expr->pos.end = expr->comma[buf_len(expr->comma) - 1]->pos.end;
		return expr;
	}
	return left;
}
static Expression* parse_expr1(void) { return expr_comma(); }
Expression* parse_expr(void) {
	Expression* expr = parse_expr1();
	if (!expr) return NULL;
	lexer_expect(TK_EOF);
	return expr;
}


void print_expr(const Expression* expr, FILE* file) {
	if (!expr) return;
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
	case EXPR_STRING:
		fprintf(file, "\"%s\"", expr->str);
		break;
	case EXPR_COMMA:
		print_expr(expr->comma[0], file);
		for (size_t i = 1; i < buf_len(expr->comma); ++i) {
			fputs(", ", file);
			print_expr(expr->comma[i], file);
		}
		break;
	case EXPR_ARRAY:
		fputc('[', file);
		print_expr(expr->comma[0], file);
		for (size_t i = 1; i < buf_len(expr->comma); ++i) {
			fputs(", ", file);
			print_expr(expr->comma[i], file);
		}
		fputc(']', file);
		break;
	case EXPR_CONDITIONAL:
		print_expr(expr->conditional.cond, file);
		fputs(" ? ", file);
		print_expr(expr->conditional.true_case, file);
		fputs(" : ", file);
		print_expr(expr->conditional.false_case, file);
		break;
	case EXPR_AT:
		print_expr(expr->at.base, file);
		fputc('[', file);
		print_expr(expr->at.index, file);
		fputc(']', file);
		break;
	}
}

void free_expr(Expression* expr) {
	if (!expr) return;
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
	case EXPR_COMMA:
	case EXPR_ARRAY:
		for (size_t i = 0; i < buf_len(expr->comma); ++i)
			free_expr(expr->comma[i]);
		buf_free(expr->comma);
		break;
	case EXPR_CONDITIONAL:
		free_expr(expr->conditional.cond);
		free_expr(expr->conditional.true_case);
		free_expr(expr->conditional.false_case);
		break;
	case EXPR_AT:
		free_expr(expr->at.base);
		free_expr(expr->at.index);
		break;
	default: break;
	}
	free(expr);
}