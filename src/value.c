#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "strint.h"
#include "value.h"

value_t make_integer_value(tkint_t i) {
	value_t val = make_value(VALUE_INTEGER);
	val.iVal = i;
	return val;
}
value_t make_float_value(tkfloat_t f) {
	value_t val = make_value(VALUE_FLOAT);
	val.fVal = f;
	return val;
}
value_t make_string_value(const char* str) {
	value_t val = make_value(VALUE_STRING);
	val.str = strint(str);
	return val;
}

value_t value_neg(value_t v) {
	switch (v.type) {
	case VALUE_EMPTY:
	case VALUE_STRING:
	case VALUE_INVALID: return make_value(VALUE_INVALID);
	case VALUE_INTEGER: return make_integer_value(-v.iVal);
	case VALUE_FLOAT:   return make_float_value(-v.fVal);
	}
}
#define ch1(t) (((a) == (t)) || ((b) == (t)))
static enum value_type infere_type(enum value_type a, enum value_type b) {
	if (a == b) return a;
	else if (ch1(VALUE_FLOAT) && ch1(VALUE_INTEGER)) return VALUE_FLOAT;
	else if (ch1(VALUE_STRING) && (ch1(VALUE_INTEGER) || ch1(VALUE_FLOAT))) return VALUE_STRING;
	else return VALUE_INVALID;
}
#undef ch1
value_t value_cast(value_t val, enum value_type type) {
	if (val.type == type) return copy_value(val);
	switch (type) {
	case VALUE_EMPTY:   return make_value(VALUE_EMPTY);
	case VALUE_INTEGER: {
		switch (val.type) {
		case VALUE_FLOAT:   return make_integer_value((tkint_t)val.fVal);
		default:            return make_value(VALUE_INVALID);
		}
	}
	case VALUE_FLOAT: {
		switch (val.type) {
		case VALUE_INTEGER: return make_float_value((tkfloat_t)val.iVal);
		default:            return make_value(VALUE_INVALID);
		}
	}
	default:            return make_value(VALUE_INVALID);
	}
}
value_t value_add(const value_t a, const value_t b) {
	switch (a.type) {
	case VALUE_INTEGER: {
		switch (b.type) {
		case VALUE_INTEGER: return make_integer_value(a.iVal + b.iVal);
		case VALUE_FLOAT:   return make_float_value(a.iVal + b.fVal);
		case VALUE_STRING: {
			const size_t a_len = (size_t)log10l(llabs(a.iVal)) + 2;
			const size_t b_len = strlen(b.str);
			char* str = (char*)malloc(a_len + b_len + 1);
			snprintf(str, a_len, "%jd", a.iVal);
			memcpy(str + strlen(str), b.str, b_len);
			str[a_len + b_len] = '\0';
			const value_t val = make_string_value(str);
			free(str);
			return val;
		}
		default:            return make_value(VALUE_INVALID);
		}
	}
	case VALUE_FLOAT: {
		switch (b.type) {
		case VALUE_INTEGER: return make_float_value(a.fVal + b.iVal);
		case VALUE_FLOAT:   return make_float_value(a.fVal + b.fVal);
		case VALUE_STRING: {
			const size_t a_len = (size_t)log10l(fabs(a.fVal)) + 10;
			const size_t b_len = strlen(b.str);
			char* str = (char*)malloc(a_len + b_len + 1);
			snprintf(str, a_len, "%f", a.fVal);
			memcpy(str + strlen(str), b.str, b_len);
			str[a_len + b_len] = '\0';
			const value_t val = make_string_value(str);
			free(str);
			return val;
		}
		default:            return make_value(VALUE_INVALID);
		}
	}
	case VALUE_STRING: {
		switch (b.type) {
		case VALUE_INTEGER: {
			const size_t a_len = strlen(a.str);
			const size_t b_len = (size_t)log10l(llabs(b.iVal)) + 2;
			char* str = (char*)malloc(a_len + b_len + 1);
			memcpy(str, a.str, a_len);
			snprintf(str + a_len, b_len, "%jd", b.iVal);
			str[a_len + b_len] = '\0';
			const value_t val = make_string_value(str);
			free(str);
			return val;
		}
		case VALUE_FLOAT: {
			const size_t a_len = strlen(a.str);
			const size_t b_len = (size_t)log10l(fabs(b.fVal)) + 10;
			char* str = (char*)malloc(a_len + b_len + 1);
			memcpy(str, a.str, a_len);
			snprintf(str + a_len, b_len, "%f", b.fVal);
			str[a_len + b_len] = '\0';
			const value_t val = make_string_value(str);
			free(str);
			return val;
		}
		case VALUE_STRING: {
			const size_t a_len = strlen(a.str);
			const size_t b_len = strlen(b.str);
			char* str = (char*)malloc(a_len + b_len + 1);
			memcpy(str, a.str, a_len);
			memcpy(str + a_len, b.str, b_len);
			str[a_len + b_len] = '\0';
			const value_t val = make_string_value(str);
			free(str);
			return val;
		}
		default:            return make_value(VALUE_INVALID);
		}
	}
	default:                return make_value(VALUE_INVALID);
	}
}
value_t value_sub(const value_t xa, const value_t xb) {
	const value_t a = value_cast(xa, infere_type(xa.type, xb.type));
	const value_t b = value_cast(xb, infere_type(xa.type, xb.type));
	if (a.type != b.type) return make_value(VALUE_INVALID);
	switch (a.type) {
	case VALUE_INTEGER: return make_integer_value(a.iVal - b.iVal);
	case VALUE_FLOAT:   return make_float_value(a.fVal - b.fVal);
	default:            return make_value(VALUE_INVALID);
	}
}
value_t value_mul(const value_t xa, const value_t xb) {
	const value_t a = value_cast(xa, infere_type(xa.type, xb.type));
	const value_t b = value_cast(xb, infere_type(xa.type, xb.type));
	if (a.type != b.type) return make_value(VALUE_INVALID);
	switch (a.type) {
	case VALUE_INTEGER: return make_integer_value(a.iVal * b.iVal);
	case VALUE_FLOAT:   return make_float_value(a.fVal * b.fVal);
	default:            return make_value(VALUE_INVALID);
	}
}
value_t value_pow(const value_t xa, const value_t xb) {
	const value_t a = value_cast(xa, infere_type(xa.type, xb.type));
	const value_t b = value_cast(xb, infere_type(xa.type, xb.type));
	if (a.type != b.type) return make_value(VALUE_INVALID);
	switch (a.type) {
	case VALUE_INTEGER: return make_integer_value((tkint_t)pow(a.iVal, b.iVal));
	case VALUE_FLOAT:   return make_float_value(pow(a.fVal, b.fVal));
	default:            return make_value(VALUE_INVALID);
	}
}
value_t value_div(const value_t xa, const value_t xb) {
	const value_t a = value_cast(xa, infere_type(xa.type, xb.type));
	const value_t b = value_cast(xb, infere_type(xa.type, xb.type));
	if (a.type != b.type) return make_value(VALUE_INVALID);
	switch (a.type) {
	case VALUE_INTEGER: return make_integer_value(a.iVal / b.iVal);
	case VALUE_FLOAT:   return make_float_value(a.fVal / b.fVal);
	default:            return make_value(VALUE_INVALID);
	}
}

void print_value(value_t val, FILE* file) {
	switch (val.type) {
	case VALUE_EMPTY:   fputs("(empty)", file); break;
	case VALUE_INVALID: fputs("(invalid)", file); break;
	case VALUE_INTEGER: fprintf(file, "%jd", val.iVal); break;
	case VALUE_STRING:  fprintf(file, "%s", val.str); break;
	case VALUE_FLOAT:   fprintf(file, "%f", val.fVal); break;
	}
}
