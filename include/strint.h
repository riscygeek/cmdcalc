#ifndef FILE_STRINT_H
#define FILE_STRINT_H
#include <stddef.h>

//  Interned strings can be compared with == instead of strcmp()

const char* strint(const char*);            // Creates a interned string from a zero-terminated string
const char* strnint(const char*, size_t);   // Creates a interned string

#endif /* FILE_STRINT_H */
