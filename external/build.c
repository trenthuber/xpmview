#include "../build.h"

#define ROOT "../"

#include "cbs/cbs.c"

int main(void) {
	char **src;
	int i;

	build(NULL);

	src = (char *[]){RLSRCS(ROOT), NULL};
	cflags = (char *[]){RLCFPLAT, NULL};
	for (i = 0; i < 6; ++i) compile(src[i], NULL);
	cflags = (char *[]){RLCFPLAT, "-I" ROOT RLGLFWINC, NULL};
	compile(src[6], NULL);
	cflags = (char *[]){RLCFPLAT, "-I" ROOT RLGLFWINC, RLCFOBJC, NULL};
	compile(src[7], NULL);

	load('s', ROOT RLLIB, RLSRCS(ROOT), NULL);

	return 0;
}
