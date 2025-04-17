#define ROOT "../../"
#include "../../build.h"

#include "../../external/cbs/cbs.c"

int main(void) {
	build(NULL);

	cflags = (char *[]){CFRAYLIB, CFSRC, NULL};
	compile("genfont", SRC "error", RLHDR, NULL);

#ifndef RLDYNAMIC
	lflags = (char *[]){LFRAYLIB, NULL};
#endif
	load('x', "genfont", SRC "error", "genfont", RLLIB, NULL);

	if (modified(SRC "font.c", "genfont.c") || modified(SRC "font.c", "font.ttf"))
		run("genfont", (char *[]){"./genfont", NULL}, "execution", "genfont");

	return 0;
}
