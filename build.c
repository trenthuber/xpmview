#define SRCPATH "src/"
#define FONTPATH "fonts/"

#define CFLAGS "-Wall", "-Wextra", "-Wpedantic", "-Iexternal/raylib/src/", "-Ifonts/"
#define LDFLAGS "-Llib/", "-lraylib", \
                "-framework", "CoreVideo", \
                "-framework", "IOKit", \
                "-framework", "Cocoa", \
                "-framework", "GLUT", \
                "-framework", "OpenGL"

#include "external/cbs/cbs.c"

#define HDRS \
	SRCPATH "parser", \
	SRCPATH "tokenizer", \
	SRCPATH "utils", \
	SRCPATH "xpm_mode" \

#define SRCS SRCPATH "main", HDRS

#define FONTS FONTPATH "source_code_pro_font"

int main(void) {
	char **srcs, **fonts;
	int i;

	build(NULL);

	build("external/");

	srcs = CARRAY(SRCS);
	fonts = CARRAY(FONTS);

	for (i = 0; srcs[i]; ++i) CC(srcs[i], HDRS);
	for (i = 0; fonts[i]; ++i) CC(fonts[i], fonts[i]);

	LD('x', "bin/simplexpm", SRCS, FONTS);

	return 0;
}
