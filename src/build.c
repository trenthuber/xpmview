#define ROOT "../"
#include "../build.h"

#include "../external/cbs/cbs.c"

#define CFDYEXT "-DDYEXT=\"" DYEXT "\""

#ifdef DYNAMICLIBS
#undef LFRAYLIB
#define LFRAYLIB NULL
#endif

int main(void) {
	build(NULL);

	build("colors/");
	build("font/");

	compile("colors", "colors", NULL);
	cflags = (char *[]){CFRAYLIB, NULL};
	compile("main", "cbs", "options", RLHDR, "xpm", "font.c", NULL);
	cflags = NULL;
	compile("options", "cbs", NULL);
	cflags = (char *[]){CFDYEXT, CFRAYLIB, NULL};
	compile("xpm", "cbs", "colors", RLHDR, "xpm", "colors.c", NULL);

	lflags = (char *[]){LFRAYLIB, NULL};
	load('x', SIMPLEXPM, "colors", "main", "options", "xpm", CBSLIB, RLLIB, NULL);

	return EXIT_SUCCESS;
}
