#include <sys/mman.h>

#include "../build.h"
#include "../external/cbs/cbs.c"
#include "../external/cbsfile.c"

#ifdef FONT
#include <err.h>
#include <stdlib.h>

#include "raylib.h"
#endif

#define CFRAYLIB "-I../external/raylib/src/"
#ifdef DYNAMICLIBS
#define LIBEXT DYEXT
#define LLRAYLIB NONE
#else
#define LIBEXT ".a"
#define LLRAYLIB LIST(LFRAYLIB)
#endif

#define RLLIB "../external/raylib/raylib" LIBEXT
#define CBSLIB "../external/cbs/cbs" LIBEXT

#ifdef FONT
static void buildcolors(void) {
	int quit, txtfd, codefd;
	char *txt, *code, *p;
	struct stat txtstat;
	size_t l, i;
	void *map;
	unsigned char r, g, b;

	quit = 1;
	txt = "../assets/rgb.txt";
	code = "colors.c";
	if ((txtfd = open(txt, O_RDONLY)) == -1)
		err(EXIT_FAILURE, "Unable to open `%s' for reading", txt);
	if (fstat(txtfd, &txtstat) == -1) {
		warn("Unable to stat `%s'", txt);
		goto closetxt;
	}
	l = txtstat.st_size;
	if ((p = map = mmap(NULL, l, PROT_READ | PROT_WRITE,
	                    MAP_PRIVATE, txtfd, 0)) == MAP_FAILED) {
		warn("Unable to map `%s' to memory", txt);
		goto closetxt;
	}
	if ((codefd = open(code, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1) {
		warn("Unable to open `%s' for writing", code);
		goto munmap;
	}

	if (dprintf(codefd, "#include <stddef.h>\n\n"
	            "#include \"colors.h\"\n\n"
	            "struct color colors[] = {\n"
	            "\t{\"None\", 0x00ffffff},\n") == -1) {
		warn("Unable to write to `%s'", code);
		goto closecode;
	}
	for (i = 1; p < (char *)map + l; ++i) {
		r = strtol(p, &p, 10);
		g = strtol(p, &p, 10);
		b = strtol(p, &p, 10);
		if (dprintf(codefd, "\t{\"%s\", 0x%02x%02x%02x},\n",
		            strsep(&p, "\n") + 2, r, g, b) == -1) {
			warn("Unable to write to `%s'", code);
			goto closecode;
		}
	}
	if (dprintf(codefd, "};\n\nsize_t numcolors = %zu;\n", i) == -1)
		warn("Unable to write to `%s'", code);

	quit = 0;

closecode:
	if (close(codefd) == -1) {
		warn("Unable to close `%s'", code);
		quit = 1;
	}

munmap:
	if (munmap(map, l) == -1) {
		warn("Unable to unmap memory associated with `%s'", txt);
		quit = 1;
	}

closetxt:
	if (close(txtfd) == -1) {
		warn("Unable to close `%s'", txt);
		quit = 1;
	}

	if (quit) exit(EXIT_FAILURE);
}

static void buildfont(void) {
	char *ttf, *code;
	Font font;

	ttf = "../assets/font.ttf";
	code = "font.c";

	SetTraceLogLevel(LOG_WARNING);
	InitWindow(0, 0, "");

	if (!ExportFontAsCode(font = LoadFontEx(ttf, 48, NULL, 95), code))
		errx(EXIT_FAILURE, "Unable to generate `%s' from `%s'", code, ttf);

	UnloadFont(font);
}
#endif

int main(void) {
#ifndef FONT
	cflags = LIST("-DFONT", CFRAYLIB);
	compile("build");

	lflags = LLRAYLIB;
	load('x', "build", LIST("build", RLLIB));

	run("!build", LIST("build"), "run", "build");
#else
	compile("build");
	load('x', "build", LIST("build"));

	buildcolors();
	buildfont();

	buildfiles((struct cbsfile []){{"../bin/xpmview", LLRAYLIB, 'x'},

	                               {"colors", NONE},
	                               {"main", LIST(CFRAYLIB)},
	                               {"options", NONE},
	                               {"xpm", LIST(CFRAYLIB)},

	                               {CBSLIB},
	                               {RLLIB},

	                               {NULL}});
#endif

	return EXIT_SUCCESS;
}
