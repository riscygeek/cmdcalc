#ifndef FILE_VALUE_H
#define FILE_VALUE_H
#include <stdio.h>
#include "token.h"

enum value_type {
	VALUE_INVALID,
	VALUE_EMPTY,
	VALUE_INTEGER,
	VALUE_FLOAT,
	VALUE_STRING,
};

typedef struct {
	enum value_type type;
	union {
		tkint_t iVal;
		tkfloat_t fVal;
		const char* str;
	};
} value_t;

#define make_value(t) ((value_t){.type = t})
#define copy_value(v) (v)
value_t make_integer_value(tkint_t);
value_t make_float_value(tkfloat_t);
value_t make_function_value(const char*);
value_t make_string_value(const char*);

value_t value_neg(value_t);
value_t value_cast(value_t, enum value_type);
value_t value_add(value_t, value_t);
value_t value_sub(value_t, value_t);
value_t value_mul(value_t, value_t);
value_t value_pow(value_t, value_t);
value_t value_div(value_t, value_t);

void print_value(value_t, FILE*);

#endif /* FILE_VALUE_H */
