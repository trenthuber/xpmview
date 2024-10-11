#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdbool.h>
#include <stdio.h>

#include "raylib.h"

extern FILE *file;
extern char *line_buffer;
extern char *keys;
extern unsigned int *color_table;
extern unsigned int *pixels;

bool parse_xpm_file(Image *image, const char *file_path);

#endif // _PARSER_H_
