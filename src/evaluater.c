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
	for (size_t i = 0; i < buf_len(ctx->vars); ++i)
		free_value(ctx->vars[i].value);
	buf_free(ctx->vars);
	free(ctx);
}
void evaluation_context_add_var(evaluation_context_t* ctx, const char* name, value_t* value) {
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
void evaluation_context_unset(evaluation_context_t* ctx, const char* name) {
	name = strint(name);
	for (size_t i = 0; i < buf_len(ctx->vars); ++i) {
		if (ctx->vars[i].name == name) {
			free_value(ctx->vars[i].value);
			for (size_t j = i+1; j < buf_len(ctx->vars); ++j)
				ctx->vars[j-1] = ctx->vars[j];
			buf_pop(ctx->vars);
			break;
		}
	}
}


value_t* evaluate(evaluation_context_t* ctx, const Expression* expr) {
	if (!expr) return make_value(VALUE_EMPTY);
	switch (expr->type) {
	case EXPR_INTEGER:  return make_integer_value(expr->iVal);
	case EXPR_FLOAT:    return make_float_value(expr->fVal);
	case EXPR_STRING:   return make_string_value(expr->str);
	case EXPR_PAREN:    return evaluate(ctx, expr->expr);
	case EXPR_NAME: {
		const variable_t* var = evaluation_context_get_var(ctx, expr->str);
		if (!var) error(expr->pos, "variable %s not found", expr->str);
		return var ? copy_value(var->value) : make_value(VALUE_EMPTY);
	}
	case EXPR_UNARY: {
		value_t* value = evaluate(ctx, expr->unary.expr);
		value_t* result;
		switch (expr->unary.op.type) {
		case TK_PLUS:   result = copy_value(value); break;
		case TK_MINUS:  result = value_neg(value); break;
		default:        result = make_value(VALUE_INVALID); break;
		}
		free_value(value);
		return result;
	}
	case EXPR_BINARY: {
		value_t* left = evaluate(ctx, expr->binary.left);
		value_t* right = evaluate(ctx, expr->binary.right);
		value_t* result;
		switch (expr->binary.op.type) {
		case TK_PLUS:       result = value_add(left, right); break;
		case TK_MINUS:      result = value_sub(left, right); break;
		case TK_STAR:       result = value_mul(left, right); break;
		case TK_SLASH:      result = value_div(left, right); break;
		case TK_STARSTAR:   result = value_pow(left, right); break;
		default:            result = make_value(VALUE_INVALID); break;
		}
		free_value(left);
		free_value(right);
		return result;
	}
	case EXPR_ASSIGN: {
		value_t* value = evaluate(ctx, expr->assign.expr);
		if (!errored && value)
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
			for (size_t i = 0; i < buf_len(func->userfunc.paramnames); ++i) {
				value_t* val = i < buf_len(expr->fcall.args) ? evaluate(ctx, expr->fcall.args[i]) : make_value(VALUE_EMPTY);
				evaluation_context_add_var(func_ctx, func->userfunc.paramnames[i], val);
			}
			value_t* value = evaluate(func_ctx, func->userfunc.body);
			free_evaluation_context(func_ctx);
			return value;
		}
		else {
			const size_t len = buf_len(expr->fcall.args);
			value_t** values = (value_t**)malloc(sizeof(value_t*) * len);
			for (size_t i = 0; i < len; ++i)
				values[i] = evaluate(ctx, expr->fcall.args[i]);
			value_t* result = func->native(ctx, (const value_t* const*) values, len);
			for (size_t i = 0; i < len; ++i)
				free_value(values[i]);
			free(values);
			return result;
		}
	}
	case EXPR_CONDITIONAL: {
		value_t* vcond = evaluate(ctx, expr->conditional.cond);
		bool cond;
		switch (vcond->type) {
		case VALUE_EMPTY:       cond = false; break;
		case VALUE_STRING:      cond = (strcmp(vcond->str, "true") == 0); break;
		case VALUE_INTEGER:     cond = vcond->iVal != 0; break;
		case VALUE_FLOAT:       cond = vcond->fVal != 0.0; break;
		default:                return free_value(vcond), make_value(VALUE_INVALID);
		}
		value_t* result = evaluate(ctx, cond ? expr->conditional.true_case : expr->conditional.false_case);
		free_value(vcond);
		return result;
	}
	case EXPR_COMMA:
		for (size_t i = 0; i < buf_len(expr->comma) - 1; ++i)
			evaluate(ctx, expr->comma[i]);
		return evaluate(ctx, expr->comma[buf_len(expr->comma) - 1]);
	case EXPR_AT: {
		// TODO: support for ("Hello World"[1, 2] == "el")
		value_t* array = evaluate(ctx, expr->at.base);
		if (!array || errored) return NULL;
		value_t* index = evaluate(ctx, expr->at.index);
		if (!index || errored) return free_value(array), NULL;
		if (index->type != VALUE_INTEGER) {
			error(expr->at.index->pos, "index has to be an unsigned integer!");
			free_value(array);
			free_value(index);
			return NULL;
		}
		value_t* elem = value_array_get(array, index->iVal);
		if (elem->type == VALUE_INVALID)
			error(expr->at.base->pos, "expected array or string");
		free_value(array);
		free_value(index);
		return elem;
	}
	case EXPR_ARRAY: {
		const size_t len = buf_len(expr->comma);
		value_t* array = make_array_value(len);
		for (size_t i = 0; i < len; ++i)
			buf_push(array->values, evaluate(ctx, expr->comma[i]));
		return array;
	}
	default:
		error(expr->pos, "this expression is not supported");
		return make_value(VALUE_EMPTY);
	}
}