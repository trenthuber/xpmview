#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cbs.h"
#include "options.h"
#define SUPPORT_IMAGE_EXPORT
#include "raylib.h"
#include "xpm.h"

#include "font.c"

static Texture2D *texture;

static void handleinput(void) {
	FilePathList files;
	KeyboardKey key;
	size_t len;
	static int mode = DEFAULT;

	if (IsFileDropped()) {
		files = LoadDroppedFiles();
		if (xpm) free(xpm);
		if (!(xpm = calloc(FILENAME_MAX, sizeof*xpm)))
			err(EXIT_FAILURE, "Memory allocation");
		strcpy(xpm, files.paths[0]);
		UnloadDroppedFiles(files);
		texture = reloadtexture(xpm, mode);
	} else switch ((key = GetKeyPressed())) {
	case KEY_R:
		if (xpm) texture = reloadtexture(xpm, mode);
	case KEY_NULL:
		break;
	case KEY_S:
		if (!texture) break;
		len = strlen(xpm);
		strncpy(xpm + len - 4, ".png", 4);
		ExportImage(image, xpm);
		strncpy(xpm + len - 4, ".xpm", 4);
		break;
	default:
		switch (key) {
		case KEY_M:
			mode = MODEM;
			break;
		case KEY_FOUR:
			mode = MODEG4;
			break;
		case KEY_G:
			mode = MODEG;
			break;
		case KEY_C:
			mode = MODEC;
			break;
		default:
			return;
		}
		if (xpm) texture = reloadtexture(NULL, mode);
	}
}

int main(int argc, char **argv) {
	Font font;
	char *welcome, *error;
	size_t width, height;
	float scale;
	Vector2 pos, dim;

	if (!options(argc, argv)) return EXIT_FAILURE;

	if (!debug) SetTraceLogLevel(LOG_ERROR);
	InitWindow(800, 600, "xpmview");
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(30);
	SetExitKey(KEY_Q);

	texture = reloadtexture(xpm, DEFAULT);
	if (xpm && !texture) return EXIT_FAILURE;
	font = LoadFont_Font();
	welcome = "Drag and drop an XPM file here";
	error = "Unable to parse XPM file:\n see console for details";
	while (!WindowShouldClose()) {
		handleinput();

		BeginDrawing();

		ClearBackground(CLITERAL(Color){0xec, 0xec, 0xec, 0xff});

		width = GetScreenWidth();
		height = GetScreenHeight();
		if (texture) {
			scale = width * texture->height > height * texture->width
			        ? (float)height / texture->height
			        : (float)width / texture->width;
			pos = CLITERAL(Vector2){(width - texture->width * scale) / 2,
			                        (height - texture->height * scale) / 2};
			DrawTextureEx(*texture, pos, 0, scale, WHITE);
		} else {
			dim = MeasureTextEx(font, xpm ? error : welcome, font.baseSize, 0);
			pos = CLITERAL(Vector2){(width - dim.x) / 2, (height - dim.y) / 2};
			DrawTextEx(font, xpm ? error : welcome, pos, font.baseSize, 0, BLACK);
		}

		EndDrawing();
	}

	if (xpm) free(xpm);
	if (texture) UnloadTexture(*texture);
	CloseWindow();

	return EXIT_SUCCESS;
}
