#define CBS_IMPLEMENTATION
#define CBS_LIBRARY_PATH "./cbs.d/cbs.h"
#include CBS_LIBRARY_PATH

#define CC "cc"

int main(int argc, char **argv) {
	cbs_rebuild_self(argv);
	cbs_shift_args(&argc, &argv);

	const char *raylib_src_dir = "./raylib/src";
	Cbs_File_Paths raylib_src_files = {0}, raylib_obj_files = {0};
	cbs_file_paths_build_file_ext(&raylib_src_files, raylib_src_dir, ".c");
	Cbs_Async_Procs procs = {0};
	Cbs_Cmd cmd = {0};
	cbs_file_paths_for_each(src_file, raylib_src_files) {
		char *obj_file = cbs_string_build(cbs_strip_file_ext(src_file), ".o");
		cbs_file_paths_build(&raylib_obj_files, obj_file);
		if (cbs_needs_rebuild(obj_file, src_file)) {
			cbs_cmd_build(&cmd, CC, "-c");
			if (cbs_string_eq(src_file, cbs_string_build(raylib_src_dir, "/rglfw.c")))
				cbs_cmd_build(&cmd, "-x", "objective-c");
			cbs_cmd_build(&cmd, "-DPLATFORM_DESKTOP",
			              cbs_string_build("-I", raylib_src_dir,
			                               "/external/glfw/include"));
			cbs_cmd_build(&cmd, "-o", obj_file, src_file);
			cbs_cmd_async_run(&procs, &cmd);
		}
	}
	cbs_async_wait(&procs);

	cbs_run("mkdir", "-p", "./raylib/lib");
	const char *output_file = "./raylib/lib/libraylib.a";
	if (cbs_needs_rebuild_file_paths(output_file, raylib_obj_files)) {
		cbs_cmd_build(&cmd, "ar", "-r", output_file);
		cbs_cmd_build_file_paths(&cmd, raylib_obj_files);
		cbs_cmd_run(&cmd);
	}

	return 0;
}
