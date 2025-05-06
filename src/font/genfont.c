#include <err.h>
#include <stdlib.h>

#include "raylib.h"

int main(int argc, char **argv) {
	Font font;

	if (argc != 3)
		errx(EXIT_FAILURE, "Incorrect number of arguments: %s <input.ttf> <output.c>",
		     argv[0]);

	SetTraceLogLevel(LOG_WARNING);
	InitWindow(0, 0, "");

	font = LoadFontEx(argv[1], 48, NULL, 95);
	if (!ExportFontAsCode(font, argv[2]))
		errx(EXIT_FAILURE, "Unable to generate `%s' from `%s'", argv[2], argv[1]);

	UnloadFont(font);

	/* Don't call CloseWindow() so focus doesn't shift to
	 * the transient application we open to load the font
	 */

	return EXIT_SUCCESS;
}
