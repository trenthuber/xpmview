#include "../../build.h"

#define ROOT "../../"

#include "../../external/cbs/cbs.c"

int main(void) {
	build(NULL);

	cflags = (char *[]){"-I" ROOT SRC, "-I" ROOT RLSRC, NULL};
	compile("genfont", ROOT SRC "error", ROOT RLHDR, NULL);
	lflags = (char *[]){RLLFLAGS, NULL};
	load('x', "genfont", ROOT SRC "error", "genfont", ROOT RLLIB, NULL);

	if (modified(ROOT SRC "font.c", "genfont.c") || modified(ROOT SRC "font.c", "font.ttf"))
		run("genfont", (char *[]){"./genfont", NULL}, "execution", "genfont");

	return 0;
}
