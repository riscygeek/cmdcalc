#include <stdbool.h>
#include <math.h>
#include "strint.h"
#include "token.h"
#include "buf.h"

const char* token_type_names[NUM_TOKENS] = {
	[TK_DUMMY] = "dummy",
	[TK_INTEGER] = "integer",
	[TK_FLOAT] = "float",
	[TK_NAME] = "name",
	[TK_STRING] = "string",
	[TK_PLUS] = "+",
	[TK_MINUS] = "-",
	[TK_STAR] = "*",
	[TK_STARSTAR] = "**",
	[TK_SLASH] = "/",
	[TK_LPAREN] = "(",
	[TK_RPAREN] = ")",
	[TK_LBRACK] = "[",
	[TK_RBRACK] = "]",
	[TK_COMMA] = ",",
	[TK_EQUALS] = "=",
	[TK_QMARK] = "?",
	[TK_COLON] = ":",
	[TK_EOF] = "end of file",
	[TK_ERROR] = "error"
};

void print_token(const Token tk, FILE* file) {
	switch (tk.type) {
	case TK_INTEGER:    fprintf(file, "%ju", tk.uVal); break;
	case TK_FLOAT:      fprintf(file, "%f", tk.fVal); break;
	case TK_NAME:       fputs(tk.str, file); break;
	case TK_STRING:     fprintf(file, "\"%s\"", tk.str); break;
	default:            fputs(token_type_names[tk.type], file); break;
	}
}

static char* strnrvs(char* buf, size_t begin, size_t end) {
	while (begin < end) {
		const char tmp = buf[begin];
		buf[begin] = buf[end];
		buf[end] = tmp;
		++begin;
		--end;
	}
	return buf;
}

const char* parse_int(tkint_t value) {
	if (!value) return strint("0");
	char* buf = NULL;
	bool negative = false;
	if (value < 0) {
		negative = true;
		value = -value;
	}
	while (value) {
		buf_push(buf, (value % 10) + '0');
		value /= 10;
	}
	if (negative) buf_push(buf, '-');
	strnrvs(buf, 0, buf_len(buf)-1);
	const char* str = strnint(buf, buf_len(buf));
	buf_free(buf);
	return str;
}
const char* parse_float(tkfloat_t value) {
	const size_t num_digits = 6;
	const size_t len = ceil(log10(fabs(value))) + num_digits + 2;
	char* buf = (char*)malloc(len + 1);
	snprintf(buf, len, "%.*f", (int)num_digits, value);
	const char* str = strint(buf);
	free(buf);
	return str;
}