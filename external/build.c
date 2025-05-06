#define ROOT "../"
#include "../build.h"

#include "cbs/cbs.c"

#define CBSSRC CBSROOT "cbs"
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
#ifdef DYNAMICLIBS
#define CFPIC "-fPIC"
#else
#define CFPIC NULL
#endif
#define CFGLOBALS "-DPLATFORM_DESKTOP", CFPIC

#ifndef DYNAMICLIBS
#undef LFRAYLIB
#define LFRAYLIB NULL
#endif

#ifdef DYNAMICLIBS
#define LIBTYPE 'd'
#else
#define LIBTYPE 's'
#endif

void cbs(void) {
	cflags = (char *[]){CFPIC, NULL};
	compile(CBSSRC, NULL);
	load(LIBTYPE, CBSLIB, CBSSRC, NULL);
}

void raylib(void) {
	char **src;
	size_t i;

	src = (char *[]){RLSRCS, NULL};
	cflags = (char *[]){CFGLOBALS, NULL};
	for (i = 0; i < 6; ++i) compile(src[i], NULL);
	cflags = (char *[]){CFGLFW, CFGLOBALS, NULL};
	compile(src[6], NULL);
	cflags = (char *[]){CFGRAPHICS, CFGLFW, CFGLOBALS, NULL};
	compile(src[7], NULL);

	lflags = (char *[]){LFRAYLIB, NULL};
	load(LIBTYPE, RLLIB, RLSRCS, NULL);
}

int main(void) {
	build(NULL);

	cbs();
	raylib();

	return EXIT_SUCCESS;
}
