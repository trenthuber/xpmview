#define SRCPATH "raylib/src/"

#define CFLAGS \
	"-x", "objective-c", \
	"-DPLATFORM_DESKTOP", \
	"-I" SRCPATH "external/glfw/include/"
#define LDFLAGS ""

#include "cbs/cbs.c"

#define SRCS \
	SRCPATH "raudio", SRCPATH "rcore", \
	SRCPATH "rglfw", SRCPATH "rmodels", \
	SRCPATH "rshapes", SRCPATH "rtext", \
	SRCPATH "rtextures", SRCPATH "utils"

int main(void) {
	char **srcs;
	int i;

	build(NULL);

	srcs = CARRAY(SRCS);

	for (i = 0; srcs[i]; ++i) CC(srcs[i]);

	LD('s', "../lib/raylib", SRCS);

	return 0;
}
