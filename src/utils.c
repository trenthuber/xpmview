#include <ctype.h>
#include <setjmp.h>
#include <stdio.h>
#include <string.h>

#include "utils.h"

jmp_buf env;
unsigned int line_number;

char *strstrip(char **string) {
	for (; isspace(**string) && **string != '\0'; ++*string);
	if (**string == '\0') return *string;
	char *end;
	for (end = *string + strlen(*string); isspace(*(end - 1)); --end);
	*end = '\0';
	return *string;
}
