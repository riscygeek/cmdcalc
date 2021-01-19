#ifndef FILE_VALUE_H
#define FILE_VALUE_H
#include <stdlib.h>
#include <stdio.h>
#include "token.h"

enum value_type {
	VALUE_INVALID,  // error value
	VALUE_EMPTY,    // empty, can be converted to other value_type's
	VALUE_INTEGER,  // wrapper for tkint_t
	VALUE_FLOAT,    // wrapper for tkfloat_t
	VALUE_STRING,   // wrapper for interned string
	VALUE_ARRAY,    // contains zero or more Values
};

typedef struct value {
	enum value_type type;
	union {
		tkint_t iVal;
		tkfloat_t fVal;
		const char* str;
		struct value** values;  // stretechy buffer
	};
} value_t;

value_t* make_value(enum value_type);
value_t* copy_value(const value_t*);
value_t* make_integer_value(tkint_t);
value_t* make_float_value(tkfloat_t);
value_t* make_string_value(const char*);
value_t* make_float_value_from(value_t*, tkfloat_t);
value_t* make_array_value(size_t len);

void free_value(value_t*);
value_t* value_neg(const value_t*);                     // negates value_t
value_t* value_cast(const value_t*, enum value_type);   // converts value to other type
value_t* value_add(const value_t*, const value_t*);     // a + b
value_t* value_sub(const value_t*, const value_t*);     // a - b
value_t* value_mul(const value_t*, const value_t*);     // a * b
value_t* value_div(const value_t*, const value_t*);     // a / b
value_t* value_pow(const value_t*, const value_t*);     // pow(a, b) or a**b or a^b

value_t* value_array_get(const value_t*, tkint_t);      // a[i]

void print_value(const value_t*, FILE*);                // prints value_t to file

#endif /* FILE_VALUE_H */
