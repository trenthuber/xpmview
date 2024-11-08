#ifndef _UTILS_H_
#define _UTILS_H_

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>

#include "parser.h"

#define SIMPLE_XPM_MALLOC(p, size) \
	do { \
		p = malloc(size); \
		if (p == NULL) { \
			fprintf(stderr, "simplexpm: OUT OF MEMORY: Terminating\n"); \
			exit(EXIT_FAILURE); \
		} \
		memset(p, 0, size); \
	} while (0)

#define SIMPLE_XPM_FREE(p) \
	do { \
		if (p) free(p); \
		p = NULL; \
	} while (0)

extern jmp_buf env;
extern unsigned int line_number;

#define SIMPLE_XPM_ERROR(...) \
	do { \
		if (file) fclose(file); \
		SIMPLE_XPM_FREE(line_buffer); \
		SIMPLE_XPM_FREE(keys); \
		SIMPLE_XPM_FREE(color_table); \
		SIMPLE_XPM_FREE(pixels); \
		fprintf(stderr, "simplexpm: ERROR: "); \
		fprintf(stderr, __VA_ARGS__); \
		fprintf(stderr, "\n"); \
		siglongjmp(env, 1); \
	} while (0)

char *strstrip(char **string);

#endif // _UTILS_H_
