#include "stringutils.h"

#include <string.h>
#include <stdlib.h>

int str_startswith(const char* str, const char* pre)
{
	int slen = strlen(str),
		plen = strlen(pre);

	return (slen < plen) ? 0 : strncmp(str, pre, plen) == 0;
}

char** str_split(const char* str, char c, unsigned int* num)
{
	int len = strlen(str);
	int count = 1;

	char** parts = malloc(1 * sizeof(char*));

	int ss = 0, lc = 0;
	for (int i = 0; i <= len; i++)
	{
		if (str[i] == c || str[i] == '\0')
		{
			if (count > 1)
				parts = realloc(parts, count * sizeof(char*));
			
			parts[count - 1] = (char*)calloc(lc + 1, sizeof(char));
            str_cpy(parts[count - 1], &str[ss], lc);
            char*x = parts[count - 1];

			if (str[i] == '\0')
				continue;

			ss = i + 1;
			lc = 0;
			count++;
			continue;
		}

		lc++;
	}

	*num = count;

	return parts;
}

int str_findchar(const char* str, char c)
{
	int pos = -1;

    if (str)
    {
        int len = strlen(str);
        for (int i = 0; i < len && pos == -1; i++)
        {
            if (str[i] == c)
                pos = i;
        }
    }

	return pos;
}

void str_cpy(char* dest, const char* src, unsigned long len)
{
    strncpy(dest, src, len);
    dest[len] = '\0';
}

char* str_unwrap(const char* str)
{
	int start = str_findchar(str, '<');
	int end = str_findchar(str, '>');
	unsigned long len = (unsigned long)(end - start - 1);
	char* result = calloc(len + 1, sizeof(char));
	str_cpy(result, &str[start + 1], len);
	return result;
}

void str_freearray(char** strarr, unsigned int size)
{
    for (int i = 0; i < size; i++)
        free(strarr[i]);
    
    free(strarr);
}
