#include "build.h"

#include "external/cbs/cbs.c"

int main(void) {
	build(NULL);

	build("external/");
	build("src/");

	return 0;
}
