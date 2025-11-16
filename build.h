// Rebuild with the following #define to use Raylib as a dynamic library
#define DYNAMICLIBS

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
