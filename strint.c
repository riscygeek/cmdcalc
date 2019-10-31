#define _CRT_SECURE_NO_WARNINGS
#include <string.h>
#include "strint.h"
#include "buf.h"

struct intern_entry {
	const char* str;
	size_t len;
};

static struct intern_entry* interns = NULL;

const char* strint(const char* str) {
	if (!str) return NULL;
	const size_t len = strlen(str);
	if (!len) return "";
	const size_t interns_len = buf_size(interns);
	for (size_t i = 0; i < interns_len; ++i) {
		if (interns[i].len == len && strcmp(str, interns[i].str) == 0) return interns[i].str;
	}
	char* buf = (char*)malloc(len + 1);
	if (!buf) return NULL;
	strcpy(buf, str);
	buf_push(interns, ((struct intern_entry){ buf, len }));
	return buf;
}
const char* strnint(const char* str, size_t len) {
	if (!str) return NULL;
	if (!len) return "";
	const size_t interns_len = buf_size(interns);
	len = min(len, strnlen(str, len));
	for (size_t i = 0; i < interns_len; ++i) {
		if (interns[i].len == len && strncmp(str, interns[i].str, len) == 0) return interns[i].str;
	}
	char* buf = (char*)malloc(len + 1);
	if (!buf) return NULL;
	memcpy(buf, str, len);
	buf[len] = '\0';
	buf_push(interns, ((struct intern_entry){ buf, len }));
	return buf;
}
const char* strrint(const char* begin, const char* end) {
	return begin < end ? strnint(begin, end - begin) : "";
}
void strint_free(void) {
	buf_free(interns);
}