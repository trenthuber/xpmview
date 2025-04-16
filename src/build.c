#define ROOT "../"
#include "../build.h"

#define BIN ROOT "bin/"
#define CBS ROOT "external/cbs/"

#define CFCBS "-I" CBS

#include "../external/cbs/cbs.c"

int main(void) {
	build(NULL);

	cflags = (char *[]){CFGLOBALS, NULL};
	compile("error", NULL);

	build("colors/");
	build("font/");

	cflags = (char *[]){CFGLOBALS, CFRAYLIB, NULL};
	compile("main", "error", RLHDR, "xpm", "font.c", NULL);
	cflags = (char *[]){CFGLOBALS, NULL};
	compile("options", "error", NULL);
	cflags = (char *[]){CFGLOBALS, CFRAYLIB, CFCBS, NULL};
	compile("xpm", "error", RLHDR, "xpm", CBS "cbs.c", "colors.c", NULL);

#ifndef RLDYNAMIC
	lflags = (char *[]){LFRAYLIB, NULL};
#endif
	load('x', BIN "simplexpm", "error", "main", "options", "xpm", RLLIB, NULL);

	return 0;
}
