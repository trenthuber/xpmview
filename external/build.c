#define ROOT "../"
#include "../build.h"

#define RLSRCS \
	RLSRC "raudio", RLSRC "rmodels", \
	RLSRC "rshapes", RLSRC "rtext", \
	RLSRC "rtextures", RLSRC "utils", \
	RLSRC "rcore", RLSRC "rglfw"

#ifdef __APPLE__
#define CFGRAPHICS "-x", "objective-c"
#else
#define CFGRAPHICS "-D_GLFW_X11"
#endif
#define CFGLFW "-I" RLSRC "external/glfw/include"
#ifdef RLDYNAMIC
#define CFGLOBALS "-DPLATFORM_DESKTOP", "-fPIC"
#else
#define CFGLOBALS "-DPLATFORM_DESKTOP"
#endif

#ifdef RLDYNAMIC
#define LIBTYPE 'd'
#else
#define LIBTYPE 's'
#endif

#include "cbs/cbs.c"

int main(void) {
	char **src;
	size_t i;

	build(NULL);

	src = (char *[]){RLSRCS, NULL};
	cflags = (char *[]){CFGLOBALS, NULL};
	for (i = 0; i < 6; ++i) compile(src[i], NULL);
	cflags = (char *[]){CFGLFW, CFGLOBALS, NULL};
	compile(src[6], NULL);
	cflags = (char *[]){CFGRAPHICS, CFGLFW, CFGLOBALS, NULL};
	compile(src[7], NULL);

	lflags = (char *[]){LFEXTERNAL, NULL};
	load(LIBTYPE, RLLIB, RLSRCS, NULL);

	return EXIT_SUCCESS;
}
