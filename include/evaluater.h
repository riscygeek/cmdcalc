#ifndef FILE_EVALUATER_H
#define FILE_EVALUATER_H
#include <stdbool.h>
#include "token.h"
#include "parser.h"
#include "value.h"


struct evaluation_context;
typedef struct {
	const char* name;
	value_t* value;
} variable_t;

typedef value_t*(*functiondef_t)(struct evaluation_context* ctx, const value_t* const* args, size_t num);
typedef struct {
	const char* name;
	bool user;
	union {
		functiondef_t native;
		struct {
			Expression* body;
			const char** paramnames;
		} userfunc;
	};
	bool copied;
} function_t;


typedef struct evaluation_context {
	variable_t* vars;
	function_t* funcs;
} evaluation_context_t;

evaluation_context_t* create_evaluation_context(void);
evaluation_context_t* copy_evaluation_context(const evaluation_context_t*);
void evaluation_context_add_builtins(evaluation_context_t*);
void free_evaluation_context(evaluation_context_t*);
void evaluation_context_add_var(evaluation_context_t*, const char* name, value_t* value);
const variable_t* evaluation_context_get_var(const evaluation_context_t*, const char* name);
void evaluation_context_add_func(evaluation_context_t*, const char* name, functiondef_t);
void evaluation_context_add_userfunc(evaluation_context_t*, const char* name, const char** paramnames, Expression* body);
const function_t* evaluation_context_get_func(const evaluation_context_t*, const char* name);
void evaluation_context_unset(evaluation_context_t*, const char* name);

value_t* evaluate(evaluation_context_t* ctx, const Expression* expr);

#endif /* FILE_EVALUATER_H */
