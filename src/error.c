#include <stdarg.h>
#include <stdio.h>
#include "parser.h"
#include "error.h"

#if __USE_POSIX
#include <unistd.h>
#endif

bool errored = false;

void error(tokenpos_t pos, const char* fmt, ...) {
	va_list ap;
	fprintf(stderr, "%*s^: ", 2 + pos.begin, "");
	va_start(ap, fmt);
	for (size_t i = 0; fmt[i]; ++i) {
		char ch = fmt[i];
		if (ch != '%') fputc(ch, stderr);
		else {
			if (!fmt[i + 1]) return;
			ch = fmt[++i];
			switch (ch) {
			case '%':   fputc('%', stderr); break;
			case 'c':   fputc(va_arg(ap, int), stderr); break;
			case 's':   fputs(va_arg(ap, const char*), stderr); break;
			case 'd':   fprintf(stderr, "%d", va_arg(ap, int)); break;
			case 'u':   fprintf(stderr, "%u", va_arg(ap, unsigned)); break;
			case 'p':   fprintf(stderr, "%zu", va_arg(ap, size_t)); break;
			case 't':   print_token(va_arg(ap, Token), stderr); break;
			case 'e':   print_expr(va_arg(ap, Expression*), stderr); break;
			default:    continue;
			}
		}
	}
	fputc('\n', stderr);
	errored = true;
	va_end(ap);
	fflush(stderr);
#if _POSIX_C_SOURCE
	usleep(50000);
#endif
}