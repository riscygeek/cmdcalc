#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "evaluater.h"
#include "token.h"
#include "buf.h"

#define declare(name) static value_t* builtin_##name(evaluation_context_t* ctx, const value_t* const* args, size_t num)

declare(sin) {
	if (num < 1) return make_value(VALUE_INVALID);
	value_t* val = value_cast(args[0], VALUE_FLOAT);
	return val->type == VALUE_INVALID ? val : make_float_value_from(val, sin(val->fVal));
}
declare(cos) {
	if (num < 1) return make_value(VALUE_INVALID);
	value_t* val = value_cast(args[0], VALUE_FLOAT);
	return val->type == VALUE_INVALID ? val : make_float_value_from(val, cos(val->fVal));
}
declare(tan) {
	if (num < 1) return make_value(VALUE_INVALID);
	value_t* val = value_cast(args[0], VALUE_FLOAT);
	return val->type == VALUE_INVALID ? val : make_float_value_from(val, tan(val->fVal));
}

declare(log) {
	if (num < 1) return make_value(VALUE_INVALID);
	value_t* val = value_cast(args[0], VALUE_FLOAT);
	return val->type == VALUE_INVALID ? val : make_float_value_from(val, log(val->fVal));
}
declare(log10) {
	if (num < 1) return make_value(VALUE_INVALID);
	value_t* val = value_cast(args[0], VALUE_FLOAT);
	return val->type == VALUE_INVALID ? val : make_float_value_from(val, log10(val->fVal));
}
declare(log2) {
	if (num < 1) return make_value(VALUE_INVALID);
	value_t* val = value_cast(args[0], VALUE_FLOAT);
	return val->type == VALUE_INVALID ? val : make_float_value_from(val, log2(val->fVal));
}

declare(exit) {
	int ec = 0;
	if (num >= 1) {
		value_t* val = value_cast(args[0], VALUE_INTEGER);
		if (val->type == VALUE_INVALID) return val;
		else ec = val->iVal;
	}
	exit(ec);
	return make_value(VALUE_EMPTY);
}
declare(typestr) {
	if (num < 0) return make_value(VALUE_INVALID);
	switch (args[0]->type) {
	case VALUE_INVALID: return make_string_value("invalid");
	case VALUE_STRING:  return make_string_value("string");
	case VALUE_FLOAT:   return make_string_value("float");
	case VALUE_INTEGER: return make_string_value("integer");
	case VALUE_EMPTY:   return make_string_value("empty");
	default:            return make_string_value("error");
	}
}
declare(invalid) {
	return make_value(VALUE_INVALID);
}
declare(empty) {
	return make_value(VALUE_EMPTY);
}
declare(sqrt) {
	if (num < 1) return make_value(VALUE_INVALID);
	else switch (args[0]->type) {
	case VALUE_INTEGER: return make_float_value(sqrt(args[0]->iVal));
	case VALUE_FLOAT:   return make_float_value(sqrt(args[0]->fVal));
	default:            return make_value(VALUE_INVALID);
	}
}

#define ch1(t, a, b) (((a->type) == (t)) || ((b->type) == (t)))
declare(equals) {
	if (num != 2) return make_value(VALUE_INVALID);
	if (ch1(VALUE_INVALID, args[0], args[1]))
		return make_integer_value(0);
	else if (ch1(VALUE_EMPTY, args[0], args[1]))
		return make_integer_value(args[0]->type == args[1]->type);
	else if (ch1(VALUE_STRING, args[0], args[1]))
		return make_integer_value(args[0]->type == args[1]->type ? strcmp(args[0]->str, args[1]->str) == 0 : 0);
	else if (ch1(VALUE_FLOAT, args[0], args[1])) {
		if (ch1(VALUE_INTEGER, args[0], args[1]))
			return make_integer_value(value_cast(args[0], VALUE_FLOAT)->fVal == value_cast(args[1], VALUE_FLOAT)->fVal);
		else if (args[0]->type == args[1]->type)
			return make_integer_value(args[0]->fVal == args[1]->fVal);
		else return make_value(VALUE_INVALID);
	}
	else if (args[0]->type == args[1]->type && args[0]->type == VALUE_INTEGER)
		return make_integer_value(args[0]->iVal == args[1]->iVal);
	else return make_value(VALUE_INVALID);
}
declare(not) {
	if (num != 1) return make_value(VALUE_INVALID);
	return make_integer_value(args[0]->type == VALUE_INTEGER ? !args[0]->iVal : 0);
}
#define ch2(t, a, b) ((a)->type == (b)->type && (a)->type == (t))
declare(and) {
	if (num != 2) return make_value(VALUE_INVALID);
	else return make_integer_value(ch2(VALUE_INTEGER, args[0], args[1]) ? (args[0]->iVal && args[1]->iVal) : 0);
}
declare(or) {
	if (num != 2) return make_value(VALUE_INVALID);
	else return make_integer_value(ch2(VALUE_INTEGER, args[0], args[1]) ? (args[0]->iVal || args[1]->iVal) : 0);
}
declare(print) {
	for (size_t i = 0; i < num; ++i) {
		print_value(args[i], stdout);
	}
	putchar('\n');
	return make_value(VALUE_EMPTY);
}
declare(append) {
	if (num == 0) return make_value(VALUE_EMPTY);
	else if (args[0]->type != VALUE_ARRAY) return make_value(VALUE_INVALID);
	value_t* array = copy_value(args[0]);
	for (size_t i = 1; i < num; ++i)
		buf_push(array->values, copy_value(args[i]));
	return array;
}
declare(list_vars) {
	const size_t len = buf_len(ctx->vars);
	value_t* array = make_array_value(len);
	for (size_t i = 0; i < len; ++i) {
		value_t* val = make_array_value(2);
		buf_push(val->values, make_string_value(ctx->vars[i].name));
		buf_push(val->values, copy_value(ctx->vars[i].value));
		buf_push(array->values, val);
	}
	return array;
}
declare(length) {
	size_t result = 0;
	for (size_t i = 0; i < num; ++i) {
		switch (args[i]->type) {
		case VALUE_INTEGER:
		case VALUE_FLOAT:
			++result;
			break;
		case VALUE_STRING:
			result += strlen(args[i]->str);
			break;
		case VALUE_ARRAY:
			result += buf_len(args[i]->values);
			break;
		default: break;
		}
	}
	return make_integer_value(result);
}
declare(float) {
	if (num != 1) return make_value(VALUE_INVALID);
	switch (args[0]->type) {
	case VALUE_EMPTY:   return make_float_value(0.0);
	case VALUE_INTEGER: return make_float_value((tkfloat_t)args[0]->iVal);
	case VALUE_FLOAT:   return copy_value(args[0]);
	case VALUE_STRING:  return make_float_value(atof(args[0]->str));
	default:            return make_value(VALUE_INVALID);
	}
}
declare(integer) {
	if (num != 1) return make_value(VALUE_INVALID);
	switch (args[0]->type) {
	case VALUE_EMPTY:   return make_integer_value(0);
	case VALUE_INTEGER: return copy_value(args[0]);
	case VALUE_FLOAT:   return make_integer_value((tkint_t)args[0]->fVal);
	case VALUE_STRING:  return make_integer_value((tkint_t)atoll(args[0]->str));
	default:            return make_value(VALUE_INVALID);
	}
}
declare(string) {
	if (num != 1) return make_value(VALUE_INVALID);
	switch (args[0]->type) {
	case VALUE_STRING:  return copy_value(args[0]);
	case VALUE_EMPTY:   return make_string_value("");
	case VALUE_INVALID: return make_string_value("(invalid)");
	case VALUE_INTEGER: return make_string_value(parse_int(args[0]->iVal));
	case VALUE_FLOAT:   return make_string_value(parse_float(args[0]->fVal));
	default:            return make_value(VALUE_INVALID);
	}
}


#define add_builtin(name) evaluation_context_add_func(ctx, #name, builtin_##name)
void evaluation_context_add_builtins(evaluation_context_t* ctx) {
	add_builtin(sin);
	add_builtin(cos);
	add_builtin(tan);
	add_builtin(log);
	add_builtin(log10);
	add_builtin(log2);
	add_builtin(exit);
	add_builtin(typestr);
	add_builtin(equals);
	add_builtin(and);
	add_builtin(or);
	add_builtin(print);
	add_builtin(append);
	add_builtin(list_vars);
	add_builtin(invalid);
	add_builtin(empty);
	add_builtin(sqrt);
	add_builtin(length);
	add_builtin(float);
	add_builtin(integer);
	add_builtin(string);
}
