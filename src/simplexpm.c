#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#include "raylib.h"

#include "../external/stb/stb_c_lexer.h"

#define FILE_PATH_CAP 2048

#define SIMPLE_XPM_ERROR(...) \
	do { \
		fprintf(stderr, __VA_ARGS__); \
		fprintf(stderr, " (%s:%d)\n", __FILE__, __LINE__); \
		exit(EXIT_FAILURE); \
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
// printf("string: %s\n", *string);
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
// printf("ENTERING TERMINAL FUNCTION\n");
	bool result = true;
	while (**string == '\t' || **string == ' ') ++*string;
	*token = *string;
	while (**string != '\t' && **string != ' ') {
// printf("*string = %s\n", *string);
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
// printf("token: %s\nstring (%s) is pointing to: |%c|%d|\n", *token, *string, **string, (unsigned char)**string);
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

Image parse_xpm_file(const char *file_path) {
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
		SIMPLE_XPM_ERROR("Expected token \"static\"");
	check_next_token(&result, "char");
	check_next_token(&result, "*");

	if (isdigit(strstrip(&result)[0]))
		SIMPLE_XPM_ERROR("Incorrect C variable name");
	for (; strlen(result) != 0 && (isalnum(*result) || *result == '_'); ++result);

	check_next_token(&result, "[");
	check_next_token(&result, "]");
	check_next_token(&result, "=");
	check_next_token(&result, "{");

	// Parse values
	get_next_line(&buffer, file);
	result = buffer;
	check_next_token(&result, "\"");
	size_t width = convert_token_to_num(get_next_token(&result), 10),
	       height = convert_token_to_num(get_next_token(&result), 10),
	       num_colors = convert_token_to_num(get_next_token(&result), 10);
	char *chars_per_pixel_token,
	     *x_hotspot_token = NULL,
	     *y_hotspot_token = NULL,
	     *xpm_ext_token = NULL;
	if (!get_terminal_token(&result, &chars_per_pixel_token)) {
		if (!get_terminal_token(&result, &xpm_ext_token)) {
			x_hotspot_token = xpm_ext_token;
			xpm_ext_token = NULL;
			if (!get_terminal_token(&result, &y_hotspot_token)) {
				if (!get_terminal_token(&result, &xpm_ext_token))
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

	// NOTE: Hotspots are not implemented
	ssize_t x_hotspot = x_hotspot_token ? convert_token_to_num(x_hotspot_token, 10) : -1;
	ssize_t y_hotspot = y_hotspot_token ? convert_token_to_num(y_hotspot_token, 10) : -1;
	bool xpm_ext = xpm_ext_token != NULL;

// printf("width: %lu, height: %lu, ncolors: %lu, cpp: %lu, x_hotspot: %ld, y_hotspot: %ld, xpm_ext_token: %s\n", width, height, num_colors, chars_per_pixel, x_hotspot, y_hotspot, xpm_ext_token);

	// TODO: Create global tables from num_colors
	char *keys = NULL;
	keys = realloc(keys, num_colors * chars_per_pixel * sizeof(*keys));
	static unsigned int *color_table = NULL;
	color_table = realloc(color_table, NUM_XPM_MODES * num_colors * sizeof(*color_table));
	bool possible_modes[NUM_XPM_MODES] = {false};

	for (size_t i = 0; i < num_colors; ++i) {
		get_next_line(&buffer, file);
		result = buffer;
// printf("result: %s\n", result);
		check_next_token(&result, "\"");

		if (strlen(result) < chars_per_pixel)
			SIMPLE_XPM_ERROR("Unable to parse color line");
		strncpy(&keys[i * chars_per_pixel], result, chars_per_pixel);
		result += chars_per_pixel;
		if (*result != '\t' && *result != ' ')
			SIMPLE_XPM_ERROR("Incorrect whitespace in color line");
		*result++ = '\0';

		bool is_last_token;
		do {
			Xpm_Mode mode = convert_token_to_mode(get_next_token(&result));
			possible_modes[mode] = true;
// printf("MODE: %d, num_modes = %d\n", mode, NUM_XPM_MODES);
			char *color_str;
			is_last_token = get_terminal_token(&result, &color_str);

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
				printf("hex_value: %08x\h, color: 0x%08x\n", hex_value, color);
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

// printf("[");
// for (int i = 0; i < NUM_XPM_MODES; ++i) {
// 	if (possible_modes[i]) {
// 		printf("%d: [", i);
// 		for (int j = 0; j < num_colors; ++j) {
// 			printf("0x%X ", color_table[i * num_colors + j]);
// 		}
// 		printf("], \n");
// 
// 	}
// }
// printf("]");

	// TODO: Be able to dynamically change modes
	Xpm_Mode current_mode = XPM_MODE_COLOR;
	if (!possible_modes[current_mode])
		SIMPLE_XPM_ERROR("Current mode not supported");

	// Parse array of pixel values
	static unsigned int *pixels = NULL;
	pixels = realloc(pixels, width * height * sizeof(*pixels));
	char *temp = calloc(chars_per_pixel + 1, sizeof(*temp));
	for (int i = 0; i < height; ++i) {
		get_next_line(&buffer, file);
		result = buffer;
		check_next_token(&result, "\"");
		if (strlen(result) < width * chars_per_pixel + strlen("\","))
			SIMPLE_XPM_ERROR("String in pixels section is formatted improperly");
		for (int j = 0; j < width; ++j) {
			for (int k = 0; k < chars_per_pixel; ++k) {
				temp[k] = *result++;
			}
			for (int l = 0; l < num_colors; ++l) {
				if (strncmp(&keys[l * chars_per_pixel], temp, chars_per_pixel) == 0) {
					pixels[width * i + j] = color_table[current_mode * num_colors + l];
// printf("%X, ", pixels[width * i + j]);
				}
			}
		}
		if (*result++ != '"')
			SIMPLE_XPM_ERROR("Pixels section is not formatted correctly");
		check_next_token(&result, ",");
// printf("\n");
	}
	free(temp); // << TODO: Make sure to free all the other variables at some point

	if (xpm_ext) {
		// TODO: Parse extensions
	}

	get_next_line(&buffer, file);
	result = buffer;
	check_next_token(&result, "}");
	check_next_token(&result, ";");
	if (*strstrip(&result) != '\0')
		SIMPLE_XPM_ERROR("File can't have elements after array declaration");
	while (get_next_line(&buffer, file)) {
		result = buffer;
		if (*strstrip(&result) != '\0')
			SIMPLE_XPM_ERROR("File can't have elements after array declaration");
	}

	fclose(file);

	// Create Raylib Image object
	// PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,      // 32 bpp
// typedef struct Image {
//     void *data;             // Image raw data
//     int width;              // Image base width
//     int height;             // Image base height
//     int mipmaps;            // Mipmap levels, 1 by default
//     int format;             // Data format (PixelFormat type)
// } Image;

	return (Image){
		.data = pixels,
		.width = width,
		.height = height,
		.mipmaps = 1,
		.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
	};
}

int main(int argc, char **argv) {
	char file_path[FILE_PATH_CAP] = {0};
	Image image = {0};

	// Check if a file was offered on the command line
	if (argc >= 2) {
		strncpy(file_path, argv[2], FILE_PATH_CAP);
		image = parse_xpm_file(file_path);
	}

	int screenWidth = 800;
	int screenHeight = 600;
	SetTraceLogLevel(LOG_WARNING);
	InitWindow(screenWidth, screenHeight, "Hello, Raylib!");
	SetTargetFPS(60);
	while (!WindowShouldClose()) {
		if (IsFileDropped()) {
			FilePathList file_paths = LoadDroppedFiles();
			strncpy(file_path, file_paths.paths[0], FILE_PATH_CAP);
			UnloadDroppedFiles(file_paths);
			image = parse_xpm_file(file_path);
			// for (int i = 0; i < image.height; ++i) {
			// 	for (int j = 0; j < image.width; ++j) {
			// 		printf("0x%08x, ", ((unsigned int *)image.data)[j * image.width + i]);
			// 	}
			// 	printf("\n");
			// }
		}

		BeginDrawing();
		ClearBackground(RAYWHITE);

		if (image.width > 0 && image.height > 0) {
			// TODO: Need to debug the array of pixels??
			
			Texture2D texture = LoadTextureFromImage(image);
            DrawTexture(texture, screenWidth / 2 - texture.width / 2, screenHeight / 2 - texture.height / 2, WHITE);
		} else if (image.width == -1 && image.height == -1) {
			DrawText("Couldn't parse .xpm file provided", GetScreenWidth() / 2, GetScreenHeight() / 2, 24, RED);
			// Use image.data as a string to print the error message
		} else {
			DrawText("Drag and drop .xpm files here", GetScreenWidth() / 2, GetScreenHeight() / 2, 24, RED);
		}

		EndDrawing();
	}
	return 0;
}
