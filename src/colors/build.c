#define ROOT "../../"
#include "../../build.h"

#include "../../external/cbs/cbs.c"

#define COLORS SRC "colors.c"

int main(void) {
	build(NULL);

	compile("gencolors", NULL);

	load('x', "gencolors", "gencolors", NULL);

	if (modified(COLORS, "gencolors.c") || modified(COLORS, "rgb.txt"))
		run("gencolors", (char *[]){"./gencolors", "rgb.txt", COLORS, NULL},
		    "run", "gencolors");

	return EXIT_SUCCESS;
}
