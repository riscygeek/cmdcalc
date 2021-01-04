#ifndef FILE_ERROR_H
#define FILE_ERROR_H
#include <stdbool.h>
#include "token.h"

extern bool errored;

/*
 * %%
 * %p
 * %t
 */
void error(tokenpos_t pos, const char* fmt, ...);

#endif /* FILE_ERROR_H */
