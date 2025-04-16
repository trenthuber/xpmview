// #define RLDYNAMIC

#ifdef RLDYNAMIC
#define RLEXT DYEXT
#else
#define RLEXT ".a"
#endif

#define SRC ROOT "src/"

#define RLROOT ROOT "external/raylib/"
#define RLSRC RLROOT "src/"
#define RLHDR RLSRC "raylib"
#define RLLIB RLROOT "raylib" RLEXT

#define CFGLOBALS "-Wall", "-Wextra", "-Wpedantic"
#define CFRAYLIB "-I" RLSRC
#define CFSRC "-I" SRC

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
