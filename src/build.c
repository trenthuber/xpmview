#include "../build.h"

#define ROOT "../"

#include "../external/cbs/cbs.c"

int main(void) {
	build(NULL);

	compile("error", NULL);

	build("colors/");
	build("font/");

	cflags = (char *[]){"-I" ROOT RLSRC, NULL};
	compile("main", "error", ROOT RLHDR, "xpm", "font.c", NULL);
	cflags = NULL;
	compile("options", "error", NULL);
	cflags = (char *[]){"-I" ROOT RLSRC, "-I" ROOT CBSSRC, NULL};
	compile("xpm", "error", ROOT RLHDR, "xpm", ROOT CBS, "colors.c", NULL);

	lflags = (char *[]){RLLFLAGS, NULL};
	load('x', ROOT BIN "simplexpm", "error", "main", "options", "xpm", ROOT RLLIB, NULL);

	return 0;
}
