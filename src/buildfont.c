#include <err.h>
#include <stdlib.h>

#include "raylib.h"

int main(void) {
	char *ttf, *code;
	Font font;

	ttf = "../assets/font.ttf";
	code = "font.c";

	SetTraceLogLevel(LOG_WARNING);
	InitWindow(0, 0, "");

	if (!ExportFontAsCode(font = LoadFontEx(ttf, 48, NULL, 95), code))
		errx(EXIT_FAILURE, "Unable to generate `%s' from `%s'", code, ttf);

	UnloadFont(font);

	return EXIT_SUCCESS;
}
