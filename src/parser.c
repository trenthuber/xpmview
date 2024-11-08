#include <ctype.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "raylib.h"
#include "tokenizer.h"
#include "utils.h"
#include "xpm_mode.h"

FILE *file;
char *line_buffer;
char *keys;
unsigned int *color_table;
unsigned int *pixels;

bool parse_xpm_file(Image *image, const char *file_path) {
	line_number = 0;
	switch (sigsetjmp(env, 0)) {
	case 1:
		return false;
	}

	if ((file = fopen(file_path, "r")) == NULL)
		SIMPLE_XPM_ERROR("Unable to open provided file");

	check_xpm_header(file);

	get_next_line_check_eof(&line_buffer, file);
	char *line_buffer_p = line_buffer;
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
	get_next_line_check_eof(&line_buffer, file);
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
		get_next_line_check_eof(&line_buffer, file);
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
				size_t r = (hex_value & 0x00ff0000) >> (2 * 8),
				       g = (hex_value & 0x0000ff00),
				       b = (hex_value & 0x000000ff) << (2 * 8),
				       a = 0xff000000;
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
		get_next_line_check_eof(&line_buffer, file);
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

	get_next_line_check_eof(&line_buffer, file);
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
