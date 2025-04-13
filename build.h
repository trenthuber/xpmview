#define BIN "bin/"
#define EXTERNAL "external/"
#define SRC "src/"

#define CBSSRC EXTERNAL "cbs/"
#define CBS CBSSRC "cbs.c"

#define RLROOT EXTERNAL "raylib/"
#define RLSRC RLROOT "src/"

#define RLSRCS(ROOT) \
	ROOT RLSRC "raudio", ROOT RLSRC "rmodels", \
	ROOT RLSRC "rshapes", ROOT RLSRC "rtext", \
	ROOT RLSRC "rtextures", ROOT RLSRC "utils", \
	ROOT RLSRC "rcore", ROOT RLSRC "rglfw"
#define RLCFPLAT "-DPLATFORM_DESKTOP"
#define RLGLFWINC RLSRC "external/glfw/include"
#define RLCFOBJC "-x", "objective-c"

#define RLHDR RLSRC "raylib"
#define RLLIB RLROOT "raylib.a"
#define RLLFLAGS \
	"-framework", "Cocoa", \
	"-framework", "CoreVideo", \
	"-framework", "GLUT", \
	"-framework", "IOKit", \
	"-framework", "OpenGL"
