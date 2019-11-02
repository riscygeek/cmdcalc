#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#ifdef __cplusplus
extern "C" {
#endif

void outchar(char);
void outstr(const char*);
void outform(const char*, ...);
char inchar(void);
char* readline(void);

#ifdef __cplusplus
}
#endif

#endif /* __CONSOLE_H__ */