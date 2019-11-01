#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include "strint.h"
#include "parser.h"
#include "buf.h"

static char* readline(void) {
	char* buf = NULL;
	char ch;
	while ((ch = getchar()) != '\n') buf_push(buf, ch);
	buf_push(buf, '\0');
	char* str = strcpy((char*)malloc(buf_len(buf)), buf);
	buf_free(buf);
	return str;
}

int main(int argc, char** argv) {
	init_funcs();
	while (1) {
		fputs("> ", stdout);
		char* line = readline();
		if (strcmp(line, "exit") == 0) { free(line); break; }
		else if (line[0] == '\0') { free(line); continue; }
		parse(lex(line));
		free(line);
	}
	strint_free();
	return 0;
}