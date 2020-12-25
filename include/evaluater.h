#ifndef FILE_EVALUATER_H
#define FILE_EVALUATER_H
#include "token.h"
#include "parser.h"

typedef struct {
	const char* name;
	tkfloat_t value;
} variable_t;

typedef struct {
	variable_t* vars;
} evaluation_context_t;

evaluation_context_t* create_evaluation_context(void);
void free_evaluation_context(evaluation_context_t*);
void evaluation_context_add_var(evaluation_context_t*, const char* name, tkfloat_t value);
const variable_t* evaluation_context_get_var(const evaluation_context_t*, const char* name);

tkfloat_t evaluate(evaluation_context_t* ctx, const Expression* expr);

#endif /* FILE_EVALUATER_H */
