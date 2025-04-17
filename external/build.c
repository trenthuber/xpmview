#define ROOT "../"
#include "../build.h"

#define RLSRCS \
	RLSRC "raudio", RLSRC "rmodels", \
	RLSRC "rshapes", RLSRC "rtext", \
	RLSRC "rtextures", RLSRC "utils", \
	RLSRC "rcore", RLSRC "rglfw"

#ifdef RLDYNAMIC
#define CFRLGLOBAL "-DPLATFORM_DESKTOP", "-fPIC"
#else
#define CFRLGLOBAL "-DPLATFORM_DESKTOP"
#endif
#define CFRLGLFW "-I" RLSRC "external/glfw/include"
#ifdef __APPLE__
#define CFRLX "-x", "objective-c"
#else
#define CFRLX "-D_GLFW_X11"
#endif

#ifdef RLDYNAMIC
#define RLTYPE 'd'
#else
#define RLTYPE 's'
#endif

#include "cbs/cbs.c"

int main(void) {
	char **src;
	size_t i;

	build(NULL);

	src = (char *[]){RLSRCS, NULL};
	cflags = (char *[]){CFRLGLOBAL, NULL};
	for (i = 0; i < 6; ++i) compile(src[i], NULL);
	cflags = (char *[]){CFRLGLOBAL, CFRLGLFW, NULL};
	compile(src[6], NULL);
	cflags = (char *[]){CFRLGLOBAL, CFRLGLFW, CFRLX, NULL};
	compile(src[7], NULL);

#ifdef RLDYNAMIC
	lflags = (char *[]){LFRAYLIB, NULL};
#endif
	load(RLTYPE, RLLIB, RLSRCS, NULL);

	return 0;
}
