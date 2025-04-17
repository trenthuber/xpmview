#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/time.h>
#include <unistd.h>

#include "error.h"
#include "options.h"
#define SUPPORT_IMAGE_EXPORT
#include "raylib.h"
#include "xpm.h"

extern bool isGpuReady;

#include "font.c"

int main(int argc, char **argv) {
	int debug, mode;
	char *xpm, *welcome, *error;
	Image image;
	Texture2D texture;
	Font font;
	FilePathList files;
	KeyboardKey key;
	size_t len, width, height;
	float scale;
	Vector2 pos, dim;

	if (!options(argc, argv, &debug, &xpm)) return 1;

	if (!debug) SetTraceLogLevel(LOG_ERROR);
	InitWindow(800, 600, "simplexpm");
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(30);
	SetExitKey(KEY_Q);

	image = (Image){0};
	mode = DEFAULT;
	texture = gettexture(xpm, &image, mode);
	if (xpm && texture.id == 0) return 1;
	font = LoadFont_Font();
	welcome = "Drag and drop an XPM file here";
	error = "Unable to parse XPM file:\n see console for details";
	while (!WindowShouldClose()) {
		if (IsFileDropped()) {
			files = LoadDroppedFiles();
			if (xpm) RL_FREE(xpm);
			xpm = RL_CALLOC(FILENAME_MAX, 1);
			TextCopy(xpm, files.paths[0]);
			UnloadDroppedFiles(files);
			texture = gettexture(xpm, &image, mode);
		} else switch ((key = GetKeyPressed())) {
		case KEY_R:
			if (xpm) texture = gettexture(xpm, &image, mode);
			break;
		case KEY_S:
			if (texture.id == 0) break;
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
				continue;
			}
			if (xpm) texture = gettexture(NULL, &image, mode);
		case KEY_NULL:;
		}

		BeginDrawing();

		ClearBackground(CLITERAL(Color){0xec, 0xec, 0xec, 0xff});

		width = GetScreenWidth();
		height = GetScreenHeight();
		if (texture.id) {
			scale = width * texture.height > height * texture.width
			        ? (float)height / texture.height
			        : (float)width / texture.width;
			pos = CLITERAL(Vector2){
				(width - texture.width * scale) / 2,
			    (height - texture.height * scale) / 2,
			};
			DrawTextureEx(texture, pos, 0, scale, WHITE);
		} else {
			dim = MeasureTextEx(font, xpm ? error : welcome, font.baseSize, 0);
			pos = (Vector2){
				.x = (width - dim.x) / 2,
				.y = (height - dim.y) / 2,
			};
			DrawTextEx(font, xpm ? error : welcome, pos, font.baseSize, 0, BLACK);
		}

		EndDrawing();
	}

	if (xpm) RL_FREE(xpm);

	UnloadTexture(texture);
	CloseWindow();

	return 0;
}
