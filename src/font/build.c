#define ROOT "../../"
#include "../../build.h"

#include "../../external/cbs/cbs.c"

#define FONT SRC "font.c"

#ifdef DYNAMICLIBS
#undef LFRAYLIB
#define LFRAYLIB NULL
#endif

int main(void) {
	build(NULL);

	cflags = (char *[]){CFRAYLIB, NULL};
	compile("genfont", RLHDR, NULL);

	lflags = (char *[]){LFRAYLIB, NULL};
	load('x', "genfont", "genfont", RLLIB, NULL);

	if (modified(FONT, "genfont.c") || modified(FONT, "font.ttf"))
		run("genfont", (char *[]){"./genfont", "font.ttf", FONT, NULL},
		    "run", "genfont");

	return EXIT_SUCCESS;
}
