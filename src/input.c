#include <stddef.h>
#include <string.h>
#include "strint.h"
#include "input.h"

static const char* str = NULL;
static size_t pos = 0, len = 0;

void input_init_str(const char* s) {
	str = strint(s);
	pos = 0;
	len = strlen(str);
}
void input_free(void) {}

char input_peek(void) { return pos < len ? str[pos  ] : '\0'; }
char input_next(void) { return pos < len ? str[pos++] : '\0'; }
bool input_eof(void) { return pos >= len; }
void input_skip(void) { ++pos; }
size_t input_pos(void) { return pos; }