#include "../../build.h"

#define ROOT "../../"

#include "../../external/cbs/cbs.c"

int main(void) {
	build(NULL);

	cflags = (char *[]){"-I" ROOT SRC, NULL};
	compile("gencolors", ROOT SRC "error", NULL);
	load('x', "gencolors", ROOT SRC "error", "gencolors", NULL);

	if (modified(ROOT SRC "colors.c", "gencolors.c")
	    || modified(ROOT SRC "colors.c", "rgb.txt"))
		run("gencolors", (char *[]){"./gencolors", NULL}, "execution", "gencolors");

	return 0;
}
