#define CBS_IMPLEMENTATION
#define CBS_LIBRARY_PATH "./external/cbs.d/cbs.h"
#include CBS_LIBRARY_PATH

#define CC "cc"
#define CFLAGS "-Wall", "-Wextra", "-Wpedantic", "-I./external/raylib/src", "-I./fonts"
#ifdef __MACH__
#define LDFLAGS "-L./external/raylib/lib", "-lraylib", \
                "-framework", "CoreVideo", \
                "-framework", "IOKit", \
                "-framework", "Cocoa", \
                "-framework", "GLUT", \
                "-framework", "OpenGL"
#else
#define LDFLAGS "-L./external/raylib/lib", "-lraylib"
#endif // __MACH__

int main(int argc, char **argv) {
	cbs_rebuild_self(argv);
	cbs_shift_args(&argc, &argv);

	const char *subcommand = cbs_shift_args(&argc, &argv);
	if (subcommand && cbs_string_eq(subcommand, "clean")) {
		Cbs_Cmd cmd = {0};
		cbs_cmd_build(&cmd, "rm", "-rf", "./bin");
		Cbs_File_Paths obj_files = {0};
		cbs_file_paths_build_file_ext(&obj_files, "./src", ".o");
		cbs_cmd_build_file_paths(&cmd, obj_files);
		cbs_cmd_run(&cmd);
		return 0;
	}

	cbs_subbuild("./external");

	// Generate font data as a C style array
	const char *font_src = "./fonts/source_code_pro_font.c",
	           *font_obj = cbs_string_build(cbs_strip_file_ext(font_src), ".o");
	if (cbs_needs_rebuild(font_obj, font_src))
		cbs_run(CC, CFLAGS, "-c", "-o", font_obj, font_src);

	cbs_run("mkdir", "-p", "./bin");
	Cbs_File_Paths src_files = {0};
	cbs_file_paths_build_file_ext(&src_files, "./src", ".c");
	cbs_file_paths_for_each (src_file, src_files) {
		const char *obj_file = cbs_string_build(cbs_strip_file_ext(src_file), ".o");
		if (cbs_needs_rebuild(obj_file, src_file))
		    cbs_run(CC, CFLAGS, "-c", "-o", obj_file, src_file);
	}

	Cbs_File_Paths obj_files = {0};
	cbs_file_paths_build_file_ext(&obj_files, "./src", ".o");
	cbs_file_paths_build_file_ext(&obj_files, "./fonts", ".o");
	const char *bin_file = "./bin/simplexpm";
	if (cbs_needs_rebuild_file_paths(bin_file, obj_files)) {
		Cbs_Cmd cmd = {0};
		cbs_cmd_build(&cmd, CC, "-o", bin_file);
		cbs_cmd_build_file_paths(&cmd, obj_files);
		cbs_cmd_build(&cmd, LDFLAGS);
		cbs_cmd_run(&cmd);
	}

	cbs_run(bin_file);

	return 0;
}
