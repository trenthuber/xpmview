#include <ctype.h>
#include <limits.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#include "raylib.h"

#include "../external/stb/stb_c_lexer.h"

#define FILE_PATH_CAP 2048

FILE *file;
char *line_buffer;
char *keys;
unsigned int *color_table;
unsigned int *pixels;

#define SIMPLE_XPM_MALLOC(p, size) \
	do { \
		p = malloc(size); \
		if (p == NULL) { \
			fprintf(stderr, "simplexpm: OUT OF MEMORY (buy more RAM?)"); \
			exit(EXIT_FAILURE); \
		} \
		memset(p, 0, size); \
	} while (0)

#define SIMPLE_XPM_FREE(p) \
	do { \
		if (p) free(p); \
		p = NULL; \
	} while (0)

#define ERROR_MESSAGE_CAP 2048
char error_message[ERROR_MESSAGE_CAP] = "Drag and drop .xpm files here";
jmp_buf env;

#define SIMPLE_XPM_ERROR(...) \
	do { \
		if (file) fclose(file); \
		SIMPLE_XPM_FREE(line_buffer); \
		SIMPLE_XPM_FREE(keys); \
		SIMPLE_XPM_FREE(color_table); \
		SIMPLE_XPM_FREE(pixels); \
		snprintf(error_message, ERROR_MESSAGE_CAP, __VA_ARGS__); \
		siglongjmp(env, 1); \
	} while(0)

typedef enum {
	XPM_MODE_MONO = 0,
	XPM_MODE_SYMBOLIC,
	XPM_MODE_GRAYSCALE_4,
	XPM_MODE_GRAYSCALE,
	XPM_MODE_COLOR,
	NUM_XPM_MODES,
} Xpm_Mode;

Xpm_Mode convert_token_to_mode(char *token) {
	if (strcmp(token, "m") == 0) return XPM_MODE_MONO;
	if (strcmp(token, "s") == 0) return XPM_MODE_SYMBOLIC;
	if (strcmp(token, "g4") == 0) return XPM_MODE_GRAYSCALE_4;
	if (strcmp(token, "g") == 0) return XPM_MODE_GRAYSCALE;
	if (strcmp(token, "c") == 0) return XPM_MODE_COLOR;
	SIMPLE_XPM_ERROR("\"%s\" is not a valid color mode", token);
}

char *strstrip(char **string) {
	for (; isspace(**string) && **string != '\0'; ++*string);
	if (**string == '\0') return *string;
	char *end;
	for (end = *string + strlen(*string); isspace(*(end - 1)); --end);
	*end = '\0';
	return *string;
}

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

void check_next_token(char **string, char *token) {
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

size_t convert_token_to_num(char *token, int base) {
	if (token == NULL)
		SIMPLE_XPM_ERROR("Expected non-null token to parse");
	errno = 0;
	char *end_p;
	long result = strtol(token, &end_p, base);
	if (*end_p != '\0' || errno == ERANGE || result < 0)
		SIMPLE_XPM_ERROR("\"%s\" is not a valid number", token);
	return (size_t)result;
}

bool parse_xpm_file(Image *image, const char *file_path) {
	switch (sigsetjmp(env, 0)) {
	case 1:
		return false;
	}

	if ((file = fopen(file_path, "r")) == NULL)
		SIMPLE_XPM_ERROR("Unable to open provided file");

	get_next_line(&line_buffer, file);
	char *line_buffer_p = line_buffer;
	check_next_token(&line_buffer_p, "/* XPM */");

	get_next_line(&line_buffer, file);
	line_buffer_p = line_buffer;
	check_next_token(&line_buffer_p, "static");
	if (!isspace(*line_buffer_p))
		SIMPLE_XPM_ERROR("Expected token \"static\"");
	check_next_token(&line_buffer_p, "char");
	check_next_token(&line_buffer_p, "*");

	if (isdigit(strstrip(&line_buffer_p)[0]))
		SIMPLE_XPM_ERROR("Incorrect C variable name");
	for (; strlen(line_buffer_p) != 0 && (isalnum(*line_buffer_p) || *line_buffer_p == '_'); ++line_buffer_p);

	check_next_token(&line_buffer_p, "[");
	check_next_token(&line_buffer_p, "]");
	check_next_token(&line_buffer_p, "=");
	check_next_token(&line_buffer_p, "{");

	// Parse values
	get_next_line(&line_buffer, file);
	line_buffer_p = line_buffer;
	check_next_token(&line_buffer_p, "\"");
	size_t width = convert_token_to_num(get_next_token(&line_buffer_p), 10),
	       height = convert_token_to_num(get_next_token(&line_buffer_p), 10),
	       num_colors = convert_token_to_num(get_next_token(&line_buffer_p), 10);
	char *chars_per_pixel_token,
	     *x_hotspot_token = NULL,
	     *y_hotspot_token = NULL,
	     *xpm_ext_token = NULL;
	if (!get_terminal_token(&line_buffer_p, &chars_per_pixel_token)) {
		if (!get_terminal_token(&line_buffer_p, &xpm_ext_token)) {
			x_hotspot_token = xpm_ext_token;
			xpm_ext_token = NULL;
			if (!get_terminal_token(&line_buffer_p, &y_hotspot_token)) {
				if (!get_terminal_token(&line_buffer_p, &xpm_ext_token))
					SIMPLE_XPM_ERROR("Too many arguments for values");
				else if (strncmp(xpm_ext_token, "XPMEXT", strlen("XPMEXT")) != 0)
					SIMPLE_XPM_ERROR("XPMEXT value not set correctly");
			}
		} else if (strncmp(xpm_ext_token, "XPMEXT", strlen("XPMEXT")) != 0)
			SIMPLE_XPM_ERROR("Must specify y_hotspot alongside x_hotspot");
	}
	size_t chars_per_pixel = convert_token_to_num(chars_per_pixel_token, 10);

	if (width == 0 || height == 0 || num_colors == 0 || chars_per_pixel == 0)
		SIMPLE_XPM_ERROR("Invalid values token");

	// TODO: Hotspots are not implemented
	ssize_t x_hotspot = x_hotspot_token ? convert_token_to_num(x_hotspot_token, 10) : -1;
	ssize_t y_hotspot = y_hotspot_token ? convert_token_to_num(y_hotspot_token, 10) : -1;
	(void)x_hotspot;
	(void)y_hotspot;

	// TODO: Extensions are not implemented
	bool xpm_ext = xpm_ext_token != NULL;

	// Parse color codes
	SIMPLE_XPM_FREE(keys);
	SIMPLE_XPM_MALLOC(keys, num_colors * chars_per_pixel * sizeof(*keys));
	SIMPLE_XPM_FREE(color_table);
	SIMPLE_XPM_MALLOC(color_table, NUM_XPM_MODES * num_colors * sizeof(*color_table));
	bool possible_modes[NUM_XPM_MODES] = {false};
	for (size_t i = 0; i < num_colors; ++i) {
		get_next_line(&line_buffer, file);
		line_buffer_p = line_buffer;
		check_next_token(&line_buffer_p, "\"");

		if (strlen(line_buffer_p) < chars_per_pixel)
			SIMPLE_XPM_ERROR("Unable to parse color line");
		strncpy(&keys[i * chars_per_pixel], line_buffer_p, chars_per_pixel);
		line_buffer_p += chars_per_pixel;
		if (*line_buffer_p != '\t' && *line_buffer_p != ' ')
			SIMPLE_XPM_ERROR("Incorrect whitespace in color line");
		*line_buffer_p++ = '\0';

		bool is_last_token;
		do {
			Xpm_Mode mode = convert_token_to_mode(get_next_token(&line_buffer_p));
			possible_modes[mode] = true;
			char *color_str;
			is_last_token = get_terminal_token(&line_buffer_p, &color_str);

			unsigned int color;
			switch(*color_str) {
			case '#':; // RGB
				size_t hex_value = convert_token_to_num(++color_str, 16);
				if (hex_value> 0xffffff)
					SIMPLE_XPM_ERROR("Hex #%s is not a valid color value", color_str);
				size_t r = (hex_value & 0x00ff0000) >> (2 * 8);
				size_t g = (hex_value & 0x0000ff00);
				size_t b = (hex_value & 0x000000ff) << (2 * 8);
				size_t a = 0xff000000;
				color = r | g | b | a;
				break;
			case '%':; // HSV
				// TODO: Parse %HSV codes
				SIMPLE_XPM_ERROR("HSV values not implemented yet");
			default:; // "Colornames"
				// TODO: Parse colornames
				// https://en.wikipedia.org/wiki/X11_color_names#Color_name_chart
				SIMPLE_XPM_ERROR("Colorname values not implemented yet");
			}
			color_table[mode * num_colors + i] = color;
		} while(!is_last_token);
	}

	// TODO: Be able to dynamically change modes
	Xpm_Mode current_mode = XPM_MODE_COLOR;
	if (!possible_modes[current_mode])
		SIMPLE_XPM_ERROR("Current mode not supported");

	// Parse array of pixel values
	SIMPLE_XPM_FREE(pixels); // Need to have the pixels still exist even when this function exits
	SIMPLE_XPM_MALLOC(pixels, width * height * sizeof(*pixels));
	char *key_buffer;
	SIMPLE_XPM_MALLOC(key_buffer, (chars_per_pixel + 1) * sizeof(*key_buffer));
	for (size_t i = 0; i < height; ++i) {
		get_next_line(&line_buffer, file);
		line_buffer_p = line_buffer;
		check_next_token(&line_buffer_p, "\"");
		if (strlen(line_buffer_p) < width * chars_per_pixel + strlen("\","))
			SIMPLE_XPM_ERROR("String in pixels section is formatted improperly");
		for (size_t j = 0; j < width; ++j) {
			for (size_t k = 0; k < chars_per_pixel; ++k) {
				key_buffer[k] = *line_buffer_p++;
			}
			for (size_t l = 0; l < num_colors; ++l) {
				if (strncmp(&keys[l * chars_per_pixel], key_buffer, chars_per_pixel) == 0) {
					pixels[width * i + j] = color_table[current_mode * num_colors + l];
				}
			}
		}
		if (*line_buffer_p++ != '"')
			SIMPLE_XPM_ERROR("Pixels section is not formatted correctly");
		check_next_token(&line_buffer_p, ",");
	}
	SIMPLE_XPM_FREE(key_buffer);
	SIMPLE_XPM_FREE(keys);
	SIMPLE_XPM_FREE(color_table);

	// TODO: Extensions are not implemented
	if (xpm_ext) {
		printf("Parsing XPM extensions\n");
	}

	get_next_line(&line_buffer, file);
	line_buffer_p = line_buffer;
	check_next_token(&line_buffer_p, "}");
	check_next_token(&line_buffer_p, ";");
	if (*strstrip(&line_buffer_p) != '\0')
		SIMPLE_XPM_ERROR("File can't have elements after array declaration");
	while (get_next_line(&line_buffer, file)) {
		line_buffer_p = line_buffer;
		if (*strstrip(&line_buffer_p) != '\0')
			SIMPLE_XPM_ERROR("File can't have elements after array declaration");
	}
	fclose(file);
	SIMPLE_XPM_FREE(line_buffer);

	*image = (Image){
		.data = pixels,
		.width = width,
		.height = height,
		.mipmaps = 1,
		.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
	};
	return true;
}

int main(int argc, char **argv) {
	char file_path[FILE_PATH_CAP] = {0};
	Texture2D texture = {0};
	bool have_texture = false;

	// Check if a file was given on the command line
	if (argc >= 2) {
		strncpy(file_path, argv[2], FILE_PATH_CAP);
		Image image = {0};
		if (!parse_xpm_file(&image, file_path))
			have_texture = false;
		else {
			UnloadTexture(texture);
			texture = LoadTextureFromImage(image);
			have_texture = true;
		}
	}

	int screenWidth = 800;
	int screenHeight = 600;
	SetTraceLogLevel(LOG_WARNING);
	InitWindow(screenWidth, screenHeight, "simplexpm");

	while (!WindowShouldClose()) {
		if (IsFileDropped()) {
			FilePathList file_paths = LoadDroppedFiles();
			strncpy(file_path, file_paths.paths[0], FILE_PATH_CAP);
			UnloadDroppedFiles(file_paths);
			Image image = {0};
			if (!parse_xpm_file(&image, file_path))
				have_texture = false;
			else {
				UnloadTexture(texture);
				texture = LoadTextureFromImage(image);
				have_texture = true;
			}
		}

		BeginDrawing();
		ClearBackground(RAYWHITE);
		if (have_texture) {
            DrawTexture(texture, screenWidth / 2 - texture.width / 2, screenHeight / 2 - texture.height / 2, WHITE);
		} else {
			DrawText(error_message, GetScreenWidth() / 2, GetScreenHeight() / 2, 24, RED);
		}
		EndDrawing();
	}

	UnloadTexture(texture);
	CloseWindow();

	return 0;
}
