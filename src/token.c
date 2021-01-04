#include "token.h"

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