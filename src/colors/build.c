#define ROOT "../../"
#include "../../build.h"

#define COLORS SRC "colors.c"

#include "../../external/cbs/cbs.c"

int main(void) {
	build(NULL);

	cflags = (char *[]){CFSRC, NULL};
	compile("gencolors", UTILS, NULL);
	load('x', "gencolors", "gencolors", UTILS, NULL);

	if (modified(COLORS, "gencolors.c") || modified(COLORS, "rgb.txt"))
		run("gencolors", (char *[]){"./gencolors", "rgb.txt", COLORS, NULL},
		    "execution", "gencolors");

	return EXIT_SUCCESS;
}
