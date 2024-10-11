#ifndef _TOKENIZER_H_
#define _TOKENIZER_H_

#include <stdbool.h>
#include <stdio.h>

int get_next_line(char **buffer, FILE *file);
void check_next_token(char **string, char *token);
char *get_next_token(char **string);
bool get_terminal_token(char **string, char **token);
size_t convert_token_to_num(char *token, int base);

#endif // _TOKENIZER_H_
