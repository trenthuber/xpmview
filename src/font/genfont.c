#include <stdio.h>
#include <stdlib.h>

#include "error.h"
#include "raylib.h"

int main(void) {
	Font font;

	SetTraceLogLevel(LOG_WARNING);
	InitWindow(0, 0, "");

	font = LoadFontEx("font.ttf", 48, NULL, 95);
	if (!ExportFontAsCode(font, "../font.c")) {
		xpmerror("Unable to generate `font.c' from `font.ttf'");
		exit(EXIT_FAILURE);
	}

	UnloadFont(font);

	/* Don't call CloseWindow() so focus doesn't shift to
	 * the transient application we open to load the font
	 */

	return 0;
}
