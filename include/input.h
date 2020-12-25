#ifndef FILE_INPUT_H
#define FILE_INPUT_H
#include <stddef.h>
#include <stdbool.h>

void input_init_str(const char* str);
void input_free(void);

char input_peek(void);
char input_next(void);
bool input_eof(void);
void input_skip(void);
size_t input_pos(void);

#endif /* FILE_INPUT_H */
