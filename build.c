#include "external/cbs/cbs.c"

int main(void) {
	build("./");

	build("external/");
	build("src/");

	return 0;
}
