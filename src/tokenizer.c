#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#include "utils.h"

// TODO: Ignore comments
int get_next_line(char **buffer, FILE *file) {
	static size_t line_cap = 0;
	errno = 0;
	if (getline(buffer, &line_cap, file) == -1) {
		if (errno != 0)
			SIMPLE_XPM_ERROR("Unable to parse provided file: expected a new line");
		return 0;
	}
	return 1;
}

void check_next_token(char **string, const char *token) {
	size_t token_len = strlen(token);
	if (strncmp(strstrip(string), token, token_len) != 0)
		SIMPLE_XPM_ERROR("Unable to parse provided file: expected token \"%s\"", token);
	*string += token_len;
}

char *get_next_token(char **string) {
	char *result;
	do {
		if ((result = strsep(string, "\t ")) == NULL)
			SIMPLE_XPM_ERROR("Couldn't parse expected next token");
	} while (*result == '\0');
	if (*string == NULL)
		SIMPLE_XPM_ERROR("Did not expect next token to be a terminal token");
	if (strlen(result) != strlen(strstrip(&result)))
		SIMPLE_XPM_ERROR("Weird whitespace characters between array tokens");
	return result;
}

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
		SIMPLE_XPM_ERROR("Couldn't parse potential last token");
	while (**string == '\t' || **string == ' ') ++*string;
	if (**string != '"') {
		result = false;
		goto defer;
	}

check_comma:
	check_next_token(string, ",");

defer:
	if (strlen(*token) != strlen(strstrip(token)))
		SIMPLE_XPM_ERROR("Weird whitespace characters between array tokens");
	return result;
}

size_t convert_token_to_num(const char *token, int base) {
	if (token == NULL)
		SIMPLE_XPM_ERROR("Expected non-null token to parse");
	errno = 0;
	char *end_p;
	long result = strtol(token, &end_p, base);
	if (*end_p != '\0' || errno == ERANGE || result < 0)
		SIMPLE_XPM_ERROR("\"%s\" is not a valid number", token);
	return (size_t)result;
}
