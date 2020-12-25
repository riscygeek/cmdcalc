#include "token.h"

const char* token_type_names[NUM_TOKENS] = {
	[TK_DUMMY] = "dummy",
	[TK_INTEGER] = "integer",
	[TK_FLOAT] = "float",
	[TK_NAME] = "name",
	[TK_PLUS] = "+",
	[TK_MINUS] = "-",
	[TK_STAR] = "*",
	[TK_STARSTAR] = "**",
	[TK_SLASH] = "/",
	[TK_LPAREN] = "(",
	[TK_RPAREN] = ")",
	[TK_COMMA] = ",",
	[TK_EQUALS] = "=",
	[TK_EOF] = "end of file"
};

void print_token(const Token tk, FILE* file) {
	switch (tk.type) {
	case TK_INTEGER:    fprintf(file, "%ju", tk.uVal); break;
	case TK_FLOAT:      fprintf(file, "%f", tk.fVal); break;
	case TK_NAME:       fputs(tk.str, file); break;
	default:            fputs(token_type_names[tk.type], file); break;
	}
}