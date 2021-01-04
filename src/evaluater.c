#include <stdlib.h>
#include <string.h>
#include "evaluater.h"
#include "strint.h"
#include "error.h"
#include "buf.h"

static void free_function(function_t* f) {
	if (f->user && !f->copied) {
		free_expr(f->userfunc.body);
		buf_free(f->userfunc.paramnames);
	}
}

evaluation_context_t* create_evaluation_context(void) {
	evaluation_context_t* ctx = (evaluation_context_t*)malloc(sizeof(evaluation_context_t));
	if (!ctx) {
		perror("out of memory");
		exit(EXIT_FAILURE);
	}
	ctx->vars = NULL;
	ctx->funcs = NULL;
	return ctx;
}
evaluation_context_t* copy_evaluation_context(const evaluation_context_t* old_ctx) {
	evaluation_context_t* ctx = create_evaluation_context();
	const size_t vars_len = buf_len(old_ctx->vars);
	const size_t funcs_len = buf_len(old_ctx->funcs);
	buf_reserve(ctx->vars, vars_len);
	buf_reserve(ctx->funcs, funcs_len);
	for (size_t i = 0; i < vars_len; ++i)
		buf_push(ctx->vars, old_ctx->vars[i]);
	for (size_t i = 0; i < funcs_len; ++i) {
		function_t f = old_ctx->funcs[i];
		f.copied = true;
		buf_push(ctx->funcs, f);
	}
	return ctx;
}
void free_evaluation_context(evaluation_context_t* ctx) {
	for (size_t i = 0; i < buf_len(ctx->funcs); ++i)
		free_function(&ctx->funcs[i]);
	buf_free(ctx->funcs);
	buf_free(ctx->vars);
	free(ctx);
}
void evaluation_context_add_var(evaluation_context_t* ctx, const char* name, value_t value) {
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
	if (!ctx || !ctx->vars || !name) return NULL;
	name = strint(name);
	for (size_t i = 0; i < buf_len(ctx->vars); ++i) {
		if (name == ctx->vars[i].name) return &ctx->vars[i];
	}
	return NULL;
}
void evaluation_context_add_func(evaluation_context_t* ctx, const char* name, functiondef_t f) {
	name = strint(name);
	function_t func;
	func.name = name;
	func.user = false;
	func.copied = false;
	func.native = f;
	for (size_t i = 0; i < buf_len(ctx->funcs); ++i) {
		if (name == ctx->funcs[i].name) {
			free_function(&ctx->funcs[i]);
			ctx->funcs[i] = func;
			return;
		}
	}
	buf_push(ctx->funcs, func);
}
void evaluation_context_add_userfunc(evaluation_context_t* ctx, const char* name, const char** paramnames, Expression* body) {
	name = strint(name);
	function_t func;
	func.name = name;
	func.user = true;
	func.copied = false;
	func.userfunc.paramnames = paramnames;
	func.userfunc.body = body;
	for (size_t i = 0; i < buf_len(ctx->funcs); ++i) {
		if (name == ctx->funcs[i].name) {
			free_function(&ctx->funcs[i]);
			ctx->funcs[i] = func;
			return;
		}
	}
	buf_push(ctx->funcs, func);
}
const function_t* evaluation_context_get_func(const evaluation_context_t* ctx, const char* name) {
	if (!ctx || !ctx->funcs || !name) return NULL;
	name = strint(name);
	for (size_t i = 0; i < buf_len(ctx->funcs); ++i) {
		if (name == ctx->funcs[i].name) return &ctx->funcs[i];
	}
	return NULL;
}


value_t evaluate(evaluation_context_t* ctx, const Expression* expr) {
	if (!expr) return make_value(VALUE_EMPTY);
	switch (expr->type) {
	case EXPR_INTEGER:  return make_integer_value(expr->iVal);
	case EXPR_FLOAT:    return make_float_value(expr->fVal);
	case EXPR_STRING:   return make_string_value(expr->str);
	case EXPR_PAREN:    return evaluate(ctx, expr->expr);
	case EXPR_NAME: {
		const variable_t* var = evaluation_context_get_var(ctx, expr->str);
		return var ? var->value : make_value(VALUE_EMPTY);
	}
	case EXPR_UNARY: {
		const value_t value = evaluate(ctx, expr->unary.expr);
		switch (expr->unary.op.type) {
		case TK_PLUS:   return copy_value(value);
		case TK_MINUS:  return value_neg(value);
		default:        return make_value(VALUE_INVALID);
		}
	}
	case EXPR_BINARY: {
		const value_t left = evaluate(ctx, expr->binary.left);
		const value_t right = evaluate(ctx, expr->binary.right);
		switch (expr->binary.op.type) {
		case TK_PLUS:       return value_add(left, right);
		case TK_MINUS:      return value_sub(left, right);
		case TK_STAR:       return value_mul(left, right);
		case TK_SLASH:      return value_div(left, right);
		case TK_STARSTAR:   return value_pow(left, right);
		default:            return make_value(VALUE_INVALID);
		}
	}
	case EXPR_ASSIGN: {
		const value_t value = evaluate(ctx, expr->assign.expr);
		evaluation_context_add_var(ctx, expr->assign.name, value);
		return make_value(VALUE_EMPTY);
	}
	case EXPR_FCALL: {
		const function_t* func = evaluation_context_get_func(ctx, expr->fcall.name);
		if (!func) {
			error(expr->pos, "function %s is not defined!", expr->fcall.name);
			return make_value(VALUE_EMPTY);
		}
		if (func->user) {
			evaluation_context_t* func_ctx = copy_evaluation_context(ctx);
			// f(x, y, z) = x * y * z;
			for (size_t i = 0; i < my_min(buf_len(expr->fcall.args), buf_len(func->userfunc.paramnames)); ++i)
				evaluation_context_add_var(func_ctx, func->userfunc.paramnames[i], evaluate(ctx, expr->fcall.args[i]));
			const value_t value = evaluate(func_ctx, func->userfunc.body);
			free_evaluation_context(func_ctx);
			return value;
		}
		else {
			const size_t len = buf_len(expr->fcall.args);
			value_t* values = (value_t*)malloc(sizeof(value_t) * len);
			for (size_t i = 0; i < len; ++i)
				values[i] = evaluate(ctx, expr->fcall.args[i]);
			return func->native(values, len);
		}
	}
	case EXPR_CONDITIONAL: {
		const value_t vcond = evaluate(ctx, expr->conditional.cond);
		bool cond;
		switch (vcond.type) {
		case VALUE_EMPTY:       cond = false; break;
		case VALUE_STRING:      cond = (strcmp(vcond.str, "true") == 0); break;
		case VALUE_INTEGER:     cond = vcond.iVal != 0; break;
		case VALUE_FLOAT:       cond = vcond.fVal != 0.0; break;
		default:                return make_value(VALUE_INVALID);
		}
		return evaluate(ctx, cond ? expr->conditional.true_case : expr->conditional.false_case);
	}
	case EXPR_COMMA:
		for (size_t i = 0; i < buf_len(expr->comma) - 1; ++i)
			evaluate(ctx, expr->comma[i]);
		return evaluate(ctx, expr->comma[buf_len(expr->comma) - 1]);
	default:
		error(expr->pos, "this expression is not supported");
		return make_value(VALUE_EMPTY);
	}
}