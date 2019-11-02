#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include "console.h"
#include "strint.h"
#include "parser.h"

int main(int argc, char** argv) {
	init_funcs();
	while (1) {
		outstr("> ");
		char* line = readline();
		if (strcmp(line, "exit") == 0) { free(line); break; }
		else if (line[0] == '\0') { free(line); continue; }
		parse(lex(line));
		free(line);
	}
	strint_free();
	return 0;
}