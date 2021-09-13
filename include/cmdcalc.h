#ifndef FILE_CMDCALC_H
#define FILE_CMDCALC_H
#include <stdbool.h>
#include <ctype.h>
#include "buf.h"

#define CMDCALC_VERSION "2.4"

struct cmdline_args {
	const char* filename;
	int exitcode;
	const char* expression;
};
struct cmdline_args parse_args(int argc, char* argv[]);

void print_help(void);

static bool isname1(const char ch) {
	return isalpha(ch) || (ch == '_');
}
static bool isname(const char ch) {
	return isalnum(ch) || (ch == '_');
}

int run_shell(void);

#endif /* FILE_CMDCALC_H */
