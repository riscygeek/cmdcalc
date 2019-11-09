#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "console.h"
#include "strint.h"
#include "buf.h"

void outchar(char ch) { putchar(ch); }
void outstr(const char* str) { fputs(str, stdout); }
void outform(const char* fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}
char inchar(void) { return getchar(); }
char* readline(void) {
	char* buf = NULL;
	char ch;
	while ((ch = getchar()) != '\n') buf_push(buf, ch);
	buf_push(buf, '\0');
	char* str = strcpy((char*)malloc(buf_len(buf)), buf);
	buf_free(buf);
	return str;
}
