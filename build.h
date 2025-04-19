/* Switch the following #define to change whether or not raylib is built as a
 * static or dynamic library; be sure to `clean' before you `build' again.
 */
// #define RLDYNAMIC

#ifdef RLDYNAMIC
#define RLEXT DYEXT
#else
#define RLEXT ".a"
#endif

#define CBSPATH ROOT "external/cbs/"
#define CBS CBSPATH "cbs.c"
#define RLROOT ROOT "external/raylib/"
#define RLLIB RLROOT "raylib" RLEXT
#define RLSRC RLROOT "src/"
#define RLHDR RLSRC "raylib"
#define SIMPLEXPM ROOT "bin/simplexpm"
#define SRC ROOT "src/"
#define UTILS SRC "utilities"

#define CFCBS "-I" CBSPATH
#define CFRAYLIB "-I" RLSRC
#define CFSRC "-I" SRC

#ifdef __APPLE__
#define RLLFLAGS \
	"-framework", "Cocoa", \
	"-framework", "CoreVideo", \
	"-framework", "GLUT", \
	"-framework", "IOKit", \
	"-framework", "OpenGL"
#else
#define RLLFLAGS "-lm"
#endif
#ifdef RLDYNAMIC
#define LFEXTERNAL RLLFLAGS
#define LFRAYLIB NULL
#else
#define LFEXTERNAL NULL
#define LFRAYLIB RLLFLAGS
#endif
