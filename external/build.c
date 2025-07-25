#include "../build.h"
#include "cbs/cbs.c"
#include "cbsfile.c"

#define CFGLFW "-I" RLSRC "external/glfw/include"
#ifdef __APPLE__
#define CFGRAPHICS "-x", "objective-c"
#else
#define CFGRAPHICS "-D_GLFW_X11"
#endif
#ifdef DYNAMICLIBS
#define CLPIC LIST("-fPIC")
#define CFGENERAL "-DPLATFORM_DESKTOP", "-fPIC"
#define LIBTYPE 'd'
#define LLRAYLIB LIST(LFRAYLIB)
#else
#define CLPIC NONE
#define CFGENERAL "-DPLATFORM_DESKTOP"
#define LIBTYPE 's'
#define LLRAYLIB NONE
#endif

#define RLLIB "raylib/raylib"
#define RLSRC "raylib/src/"

void buildcbs(void) {
	char **c, **l;

	c = cflags;
	l = lflags;

	cflags = CLPIC;
	compile("cbs/cbs");

	lflags = NONE;
	load(LIBTYPE, "cbs/cbs", LIST("cbs/cbs"));

	cflags = c;
	lflags = l;
}

void buildraylib(void) {
	buildfiles((struct cbsfile []){{RLLIB, LLRAYLIB, LIBTYPE},

	                               {RLSRC "raudio", LIST(CFGENERAL)},
	                               {RLSRC "rcore", LIST(CFGLFW, CFGENERAL)}, 
	                               {RLSRC "rglfw", LIST(CFGLFW, CFGRAPHICS, CFGENERAL)},
	                               {RLSRC "rmodels", LIST(CFGENERAL)},
	                               {RLSRC "rshapes", LIST(CFGENERAL)},
	                               {RLSRC "rtext", LIST(CFGENERAL)},
	                               {RLSRC "rtextures", LIST(CFGENERAL)},
	                               {RLSRC "utils", LIST(CFGENERAL)},

	                               {NULL}});
}

int main(void) {
	build("./");

	buildcbs();
	buildraylib();

	return EXIT_SUCCESS;
}
