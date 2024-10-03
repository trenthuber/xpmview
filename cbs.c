#define CBS_IMPLEMENTATION
#define CBS_LIBRARY_PATH "./external/cbs.d/cbs.h"
#include CBS_LIBRARY_PATH

#define CC "cc"

int main(int argc, char **argv) {
	cbs_rebuild_self(argv);
	cbs_shift_args(&argc, &argv);

	// Build our dependencies
	cbs_subbuild("./external");

	cbs_run("mkdir", "-p", "./bin");
#ifdef __MACH__
	cbs_run(CC, "-Wall", "-Wextra", "-Wpedantic",
			"-I./external/raylib/src",
			"-o", "./bin/simplexpm",
			"./src/simplexpm.c",
			"-L./external/raylib/lib",
			"-lraylib",
			"-framework", "CoreVideo",
			"-framework", "IOKit",
			"-framework", "Cocoa",
			"-framework", "GLUT",
			"-framework", "OpenGL");
#endif // __MACH__
	cbs_run("./bin/simplexpm");

	return 0;
}
