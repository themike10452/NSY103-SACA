#ifndef STRINGUTILS_H
#define STRINGUTILS_H

int str_startswith(const char* str, const char* pre);
char** str_split(const char* str, char c, unsigned int* num);
int str_findchar(const char* str, char c);
void str_cpy(char* dest, const char* src, unsigned long len);
char* str_unwrap(const char* str);
void str_freearray(char** strarr, unsigned int size);

#endif
