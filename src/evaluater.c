#include <stdlib.h>
#include <math.h>
#include "evaluater.h"
#include "strint.h"
#include "buf.h"

evaluation_context_t* create_evaluation_context(void) {
	evaluation_context_t* ctx = (evaluation_context_t*)malloc(sizeof(evaluation_context_t));
	if (!ctx) {
		perror("out of memory");
		exit(1);
	}
	ctx->vars = NULL;
	return ctx;
}
void free_evaluation_context(evaluation_context_t* ctx) {
	buf_free(ctx->vars);
	free(ctx);
}
void evaluation_context_add_var(evaluation_context_t* ctx, const char* name, tkfloat_t value) {
	name = strint(name);
	for (size_t i = 0; i < buf_len(ctx->vars); ++i) {
		if (name == ctx->vars[i].name) {
			ctx->vars[i].value = value;
			return;
		}
	}
	const variable_t var = { name, value };
	buf_push(ctx->vars, var);
}
const variable_t* evaluation_context_get_var(const evaluation_context_t* ctx, const char* name) {
	if (!ctx || !ctx->vars) return NULL;
	name = strint(name);
	for (size_t i = 0; i < buf_len(ctx->vars); ++i) {
		if (name == ctx->vars[i].name) return &ctx->vars[i];
	}
	return NULL;
}


tkfloat_t evaluate(evaluation_context_t* ctx, const Expression* expr) {
	switch (expr->type) {
	case EXPR_INTEGER:  return (double)expr->iVal;
	case EXPR_FLOAT:    return expr->fVal;
	case EXPR_PAREN:    return evaluate(ctx, expr->expr);
	case EXPR_NAME: {
		const variable_t* var = evaluation_context_get_var(ctx, expr->str);
		if (!var) {
			// TODO: error()
			printf("variable %s not found\n", expr->str);
			exit(1);
		}
		return var->value;
	}
	case EXPR_UNARY: {
		const tkfloat_t value = evaluate(ctx, expr->unary.expr);
		switch (expr->unary.op.type) {
		case TK_PLUS:   return +value;
		case TK_MINUS:  return -value;
		default:        return 0;
		}
	}
	case EXPR_BINARY: {
		const tkfloat_t left = evaluate(ctx, expr->binary.left);
		const tkfloat_t right = evaluate(ctx, expr->binary.right);
		switch (expr->binary.op.type) {
		case TK_PLUS:       return left + right;
		case TK_MINUS:      return left - right;
		case TK_STAR:       return left * right;
		case TK_SLASH:      return left / right;
		case TK_STARSTAR:   return pow(left, right);
		default:            return 0;
		}
	}
	case EXPR_ASSIGN: {
		const tkfloat_t value = evaluate(ctx, expr->assign.expr);
		evaluation_context_add_var(ctx, expr->assign.name, value);
		return value;
	}
	default:
		printf("this expression is not supported\n");
		exit(1);
	}
}