#define ROOT "../../"
#include "../../build.h"

#define FONT SRC "font.c"

#include "../../external/cbs/cbs.c"

int main(void) {
	build(NULL);

	cflags = (char *[]){CFRAYLIB, CFSRC, NULL};
	compile("genfont", UTILS, RLHDR, NULL);

	lflags = (char *[]){LFRAYLIB, NULL};
	load('x', "genfont", "genfont", UTILS, RLLIB, NULL);

	if (modified(FONT, "genfont.c") || modified(FONT, "font.ttf"))
		run("genfont", (char *[]){"./genfont", "font.ttf", FONT, NULL},
		    "execution", "genfont");

	return EXIT_SUCCESS;
}
