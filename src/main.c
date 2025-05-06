#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cbs.h"
#include "options.h"
#define SUPPORT_IMAGE_EXPORT
#include "raylib.h"
#include "xpm.h"

#include "font.c"

static void getinput(char **xp, Texture2D *tp, Image *ip) {
	FilePathList files;
	static int mode = DEFAULT;
	KeyboardKey key;
	size_t len;

	if (IsFileDropped()) {
		files = LoadDroppedFiles();
		if (*xp) RL_FREE(*xp);
		*xp = allocate(FILENAME_MAX);
		strcpy(*xp, files.paths[0]);
		UnloadDroppedFiles(files);
		*tp = gettexture(*xp, ip, mode);
	} else switch ((key = GetKeyPressed())) {
	case KEY_R:
		if (*xp) *tp = gettexture(*xp, ip, mode);
	case KEY_NULL:
		break;
	case KEY_S:
		if (tp->id == 0) break;
		len = strlen(*xp);
		strncpy(*xp + len - 4, ".png", 4);
		ExportImage(*ip, *xp);
		strncpy(*xp + len - 4, ".xpm", 4);
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
		if (*xp) *tp = gettexture(NULL, ip, mode);
	}
}

int main(int argc, char **argv) {
	int debug;
	char *xpm, *welcome, *error;
	Image image;
	Texture2D texture;
	Font font;
	size_t width, height;
	float scale;
	Vector2 pos, dim;

	if (!options(argc, argv, &debug, &xpm)) return EXIT_FAILURE;

	if (!debug) SetTraceLogLevel(LOG_ERROR);
	InitWindow(800, 600, "simplexpm");
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(30);
	SetExitKey(KEY_Q);

	image = (Image){0};
	texture = gettexture(xpm, &image, DEFAULT);
	if (xpm && texture.id == 0) return EXIT_FAILURE;
	font = LoadFont_Font();
	welcome = "Drag and drop an XPM file here";
	error = "Unable to parse XPM file:\n see console for details";
	while (!WindowShouldClose()) {
		getinput(&xpm, &texture, &image);

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

	if (xpm) free(xpm);

	UnloadTexture(texture);
	CloseWindow();

	return EXIT_SUCCESS;
}
