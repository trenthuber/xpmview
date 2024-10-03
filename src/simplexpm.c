#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#include "raylib.h"

#include "../external/stb/stb_c_lexer.h"

#define FILE_PATH_CAP 2048

#define SIMPLEXPM_ERROR(...) \
	do { \
		fprintf(stderr, __VA_ARGS__); \
		fprintf(stderr, " (%s:%d)\n", __FILE__, __LINE__); \
		exit(EXIT_FAILURE); \
	} while(0)

// NOTE: This will clobber "string"!
char *strstrip(char **string) {
	for (; isspace(**string) && **string != '\0'; ++*string);
	if (**string == '\0') return *string;
	char *end;
	for (end = *string + strlen(*string); isspace(*(end - 1)); --end);
	*end = '\0';
	return *string;
}

int get_next_line(char **buffer, FILE *file) {
	static size_t line_cap = 0;
	errno = 0;
	if (getline(buffer, &line_cap, file) == -1) {
		if (errno != 0)
			SIMPLEXPM_ERROR("Unable to parse provided file: expected a new line");
		return 0;
	}
	return 1;
}

void check_next_token(char **string, char *token) {
	size_t token_len = strlen(token);
	if (strncmp(strstrip(string), token, token_len) != 0)
		SIMPLEXPM_ERROR("Unable to parse provided file: expected token \"%s\"", token);
	*string += token_len;
}

char *get_next_token(char **string) {
	char *result;
	do
		if ((result = strsep(string, "\t ")) == NULL)
			SIMPLEXPM_ERROR("Couldn't parse expected next token");
	while (*result == '\0');
	if (strlen(result) != strlen(strstrip(&result)))
		SIMPLEXPM_ERROR("Weird whitespace characters between array tokens");
	return result;
}

bool get_terminal_token(char **string, char **token) {
	printf("ENTERING TERMINAL FUNCTION\n");
	bool result = true;
	while (**string == '\t' || **string == ' ') ++*string;
	*token = *string;
	while (**string != '\t' && **string != ' ') {
printf("*string = %s\n", *string);
		if (**string == '"') {
			*(*string)++ = '\0';
			// while (**string == '\t' || **string == ' ') ++*string;
			goto parse_comma;
		}
		++*string;
	}
	*(*string)++ = '\0';
	if (*token == *string)
		SIMPLEXPM_ERROR("Couldn't parse potential last token");
	while (**string == '\t' || **string == ' ') ++*string;
	if (**string != '"') {
		result = false;
		goto defer;
	}
parse_comma:
printf("token: %s\nstring (%s) is pointing to: |%c|%d|\n", *token, *string, **string, (unsigned char)**string);
	while (isspace(**string)) ++*string;
	if (*(*string)++ != ',')
		SIMPLEXPM_ERROR("Incorrect format of array");
	if (strlen(strstrip(string)) != 0)
		SIMPLEXPM_ERROR("Trailing characters after array element");
defer:
	if (strlen(*token) != strlen(strstrip(token)))
		SIMPLEXPM_ERROR("Weird whitespace characters between array tokens");
	return result;
}

Image *parse_xpm_file(const char *file_path) {
	FILE *file;
	if ((file = fopen(file_path, "r")) == NULL) {
		perror("Unable to open provided file");
		exit(EXIT_FAILURE);
	}
	char *buffer = NULL, *result;

	// TODO: Exit gracefully

	get_next_line(&buffer, file);
	result = buffer;
	check_next_token(&result, "/* XPM */");

	get_next_line(&buffer, file);
	result = buffer;
	check_next_token(&result, "static");
	if (!isspace(*result))
		SIMPLEXPM_ERROR("Expected token \"static\"");
	check_next_token(&result, "char");
	check_next_token(&result, "*");

	if (isdigit(strstrip(&result)[0]))
		SIMPLEXPM_ERROR("Incorrect C variable name");
	for (; strlen(result) != 0 && (isalnum(*result) || *result == '_'); ++result);

	check_next_token(&result, "[");
	check_next_token(&result, "]");
	check_next_token(&result, "=");
	check_next_token(&result, "{");

	// Parse values
	get_next_line(&buffer, file);
	result = buffer;
	check_next_token(&result, "\"");
	char *width = get_next_token(&result);
	char *height = get_next_token(&result);
	char *num_colors = get_next_token(&result);
	char *chars_per_pixel;
	char *x_hotspot = NULL;
	char *y_hotspot = NULL;
	char *xpm_ext = NULL;
	if (!get_terminal_token(&result, &chars_per_pixel)) {
		if (!get_terminal_token(&result, &xpm_ext)) {
			x_hotspot = xpm_ext;
			xpm_ext = NULL;
			if (!get_terminal_token(&result, &y_hotspot)) {
				if (!get_terminal_token(&result, &xpm_ext))
					SIMPLEXPM_ERROR("Too many arguments for values");
				else if (strncmp(xpm_ext, "XPMEXT", strlen("XPMEXT")) != 0)
					SIMPLEXPM_ERROR("XPMEXT value not set correctly");
			}
		} else if (strncmp(xpm_ext, "XPMEXT", strlen("XPMEXT")) != 0)
			SIMPLEXPM_ERROR("Must specify y_hotspot alongside x_hotspot");
	}

	printf("width: %s, height: %s, ncolors: %s, cpp: %s, x_hotspot: %s, y_hotspot: %s, xpm_ext: %s\n", width, height, num_colors, chars_per_pixel, x_hotspot, y_hotspot, xpm_ext);

	while (get_next_line(&buffer, file)) {
		result = buffer;
		// printf("%s", buffer);
	}
	fclose(file);
	return NULL;
}

int main(int argc, char **argv) {
	char file_path[FILE_PATH_CAP] = {0};
	Image *image = NULL;

	// Check if a file was offered on the command line
	if (argc >= 2) {
		strncpy(file_path, argv[2], FILE_PATH_CAP);
		image = parse_xpm_file(file_path);
	}

	InitWindow(800, 600, "Hello, Raylib!");
	SetTargetFPS(60);
	while (!WindowShouldClose()) {
		if (IsFileDropped()) {
			FilePathList file_paths = LoadDroppedFiles();
			strncpy(file_path, file_paths.paths[0], FILE_PATH_CAP);
			image = parse_xpm_file(file_path);
			UnloadDroppedFiles(file_paths);
		}

		BeginDrawing();
		ClearBackground(BLACK);

		if (image)
			;// TODO: display file_pixels
		else
			DrawText("Drag and drop .xpm files here", GetScreenWidth() / 2, GetScreenHeight() / 2, 24, RED);

		EndDrawing();
	}
	return 0;
}
