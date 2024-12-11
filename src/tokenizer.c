#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#include "utils.h"

static bool is_multiline;
static unsigned int multiline_num;

static bool line_is_comment(char *line) {
	for (int i = 0; i < (int)strlen(line); ++i) {
		switch (line[i]) {
		case '/':
			switch (line[i + 1]) {
			case '/':
				line[i] = '\0';
				goto loop_end;
			case '*':;
				char *multiline_end = strstr(&line[i + 2], "*/");
				if (!multiline_end) {
					line[i] = '\0';
					multiline_num = line_number;
					is_multiline = true;
					goto loop_end;
				}
				line[i] = ' ';
				multiline_end += strlen("*/");
				memmove(&line[i + 1], multiline_end, strlen(multiline_end) + 1);
				break;
			}
			break;
		case '*':
			if (!is_multiline) break;
			if (line[i + 1] != '/') break;
			is_multiline = false;
			memmove(line, &line[i + 2], strlen(&line[i + 2]) + 1);
			i = -1;
			break;
		case '"':;
			char *string_end = strstr(&line[i + 1], "\"");
			if (!string_end)
				SIMPLE_XPM_ERROR("String on line %d not terminated; "
								 "no support for multiline strings", line_number);
			i += string_end - &line[i];
			break;
		}
	}
loop_end:

	// Return true if the line is only whitespace at this point
	return *strstrip(&line) == '\0';
}

bool get_next_line(char **buffer, FILE *file) {
	static size_t buffer_cap = 0;
	errno = 0;
	do {
		if (getline(buffer, &buffer_cap, file) == -1) {
			if (errno != 0)
				SIMPLE_XPM_ERROR("Error occured while getting line %d", line_number + 1);
			if (is_multiline) {
				is_multiline = false;
				SIMPLE_XPM_ERROR("Failed to close multiline comment from line %d",
				                 multiline_num);
			}
			return false;
		}
		++line_number;
	} while (line_is_comment(*buffer));
	return true;
}

void get_next_line_check_eof(char **buffer, FILE *file) {
	if (!get_next_line(buffer, file))
		SIMPLE_XPM_ERROR("Expected another line after line %d", line_number);
}

void check_next_token(char **string, const char *token) {
	size_t token_len = strlen(token);
	if (strncmp(strstrip(string), token, token_len) != 0)
		SIMPLE_XPM_ERROR("Expected the word `%s' at line %d", token, line_number);
	*string += token_len;
}

void check_xpm_header(FILE *file) {
	char *buffer = NULL;
	size_t buffer_cap;
	if (getline(&buffer, &buffer_cap, file) == -1) {
		if (errno != 0)
			SIMPLE_XPM_ERROR("Error occured while getting line %d", line_number + 1);
		SIMPLE_XPM_ERROR("The provided file is empty");
	}
	++line_number;
	check_next_token(&buffer, "/* XPM */");
	if (*strstrip(&buffer) != '\0')
		SIMPLE_XPM_ERROR("Trailing text after XPM file header; incorrect format");
}

char *get_next_token(char **string) {
	char *result;
	do
		if ((result = strsep(string, "\t ")) == NULL)
			SIMPLE_XPM_ERROR("Unable to parse expected token on line %d", line_number);
	while (*result == '\0');
	if (*string == NULL) // This function doesn't expect terminal tokens
		SIMPLE_XPM_ERROR("Missing words on line %d", line_number);
	if (strlen(result) != strlen(strstrip(&result)))
		SIMPLE_XPM_ERROR("Words must be separated by tabs and/or spaces; line %d "
		                 "contains nonstandard whitespace", line_number);
	return result;
}

// Used for the "values" and "colors" sections, but not the "pixels" section
bool get_terminal_token(char **string, char **token) {
	bool result = true;
	while (**string == '\t' || **string == ' ') ++*string;
	*token = *string;
	while (**string != '\t' && **string != ' ') {
		if (**string == '"') {
			*(*string)++ = '\0';
			goto check_comma;
		}
		++*string;
	}
	*(*string)++ = '\0';
	if (*token == *string)
		SIMPLE_XPM_ERROR("Missing last word on line %d", line_number);
	while (**string == '\t' || **string == ' ') ++*string;
	if (**string != '"') {
		result = false;
		goto defer;
	}

check_comma:
	check_next_token(string, ",");

defer:
	if (strlen(*token) != strlen(strstrip(token)))
		SIMPLE_XPM_ERROR("Words must be separated by tabs and/or spaces; line %d "
		                 "contains nonstandard whitespace", line_number);
	return result;
}

size_t convert_token_to_num(const char *token, int base) {
	if (token == NULL)
		SIMPLE_XPM_ERROR("Unable to parse empty word from line %d", line_number);
	errno = 0;
	char *end_p;
	long result = strtol(token, &end_p, base);
	if (*end_p != '\0' || errno == ERANGE || result < 0) {
		if (strlen(token) == 0)
			SIMPLE_XPM_ERROR("Unable to parse an empty word from line %d as a number",
			                 line_number);
		else
			SIMPLE_XPM_ERROR("Word `%s' from line %d is not a valid number",
			                 token, line_number);
	}
	return (size_t)result;
}
