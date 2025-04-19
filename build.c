#define ROOT
#include "build.h"

#include "external/cbs/cbs.c"

int main(void) {
	build(NULL);

	build("external/");
	build("src/");

	cflags = (char *[]){CFCBS, NULL};
	compile("clean", CBS, NULL);

	load('x', "clean", "clean", NULL);

	return 0;
}
