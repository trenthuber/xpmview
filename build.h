/* Switch the following #define to change whether or not raylib is built as a
 * static or dynamic library; be sure to `clean' before you `build' again.
 */
// #define DYNAMICLIBS

#ifdef DYNAMICLIBS
#define LIBEXT DYEXT
#else
#define LIBEXT ".a"
#endif

#define CBSROOT ROOT "external/cbs/"
#define CBSLIB CBSROOT "cbs" LIBEXT
#define RLROOT ROOT "external/raylib/"
#define RLLIB RLROOT "raylib" LIBEXT
#define RLSRC RLROOT "src/"
#define RLHDR RLSRC "raylib"
#define SIMPLEXPM ROOT "bin/simplexpm"
#define SRC ROOT "src/"

#define CFRAYLIB "-I" RLSRC

#ifdef __APPLE__
#define LFRAYLIB \
	"-framework", "Cocoa", \
	"-framework", "CoreVideo", \
	"-framework", "GLUT", \
	"-framework", "IOKit", \
	"-framework", "OpenGL"
#else
#define LFRAYLIB "-lm"
#endif
