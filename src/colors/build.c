#define ROOT "../../"
#include "../../build.h"

#include "../../external/cbs/cbs.c"

int main(void) {
	build(NULL);

	cflags = (char *[]){CFGLOBALS, CFSRC, NULL};
	compile("gencolors", SRC "error", NULL);
	load('x', "gencolors", SRC "error", "gencolors", NULL);

	if (modified(SRC "colors.c", "gencolors.c")
	    || modified(SRC "colors.c", "rgb.txt"))
		run("gencolors", (char *[]){"./gencolors", NULL}, "execution", "gencolors");

	return 0;
}
