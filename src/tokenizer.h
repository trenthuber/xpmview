#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include <stdbool.h>
#include <stdio.h>

bool get_next_line(char **buffer, FILE *file);
void get_next_line_check_eof(char **buffer, FILE *file);
void check_next_token(char **string, const char *token);
void check_xpm_header(FILE *file);
char *get_next_token(char **string);
bool get_terminal_token(char **string, char **token);
size_t convert_token_to_num(const char *token, int base);

#endif // _TOKENIZER_H_
