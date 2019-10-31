#ifndef __STRINT_H__
#define __STRINT_H__
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

const char* strint(const char*);
const char* strnint(const char*, size_t);
const char* strrint(const char*, const char*);
void strint_free(void);

#ifdef __cplusplus
}
#endif

#endif /* __STRINT_H__ */