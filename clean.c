#define ROOT
#include "build.h"

#include "external/cbs/cbs.c"

#define FIND "/usr/bin/find"
#define RM "/bin/rm"
#define GENCOLORS SRC "colors/gencolors"
#define GENFONT SRC "font/genfont"
#define COLORS SRC "colors.c"
#define FONT SRC "font.c"

int main(void) {
	char *what, **whos, **regexes, ***rms;
	size_t i;
	pid_t cpid;

	what = "remove";

	// Remove build executables and object files
	whos = (char *[2]){"{*/*build,*/*/*build}", "{*/*.o,*/*/*.o,*/*/*/*.o}"};
	regexes = (char *[2]){".*[^\\.]/build$", ".*\\.o$"};
	for (i = 0; i < 2; ++i) {
		if ((cpid = fork()) == 0)
			run(FIND, (char *[]){"find", ".", "-regex", regexes[i],
			                     "-exec", "rm", "{}", "+", NULL}, what, whos[i]);
		await(cpid, what, whos[i]);
	}

	/* Remove cbs and raylib libraries, application executables,
	 * automatically generated source files, and ourself
	 */
	whos = (char *[5]){extend(CBSLIB, LIBEXT), extend(RLLIB, LIBEXT),
	                   "{" GENCOLORS "," GENFONT "," SIMPLEXPM "}",
	                   "{" COLORS "," FONT "}", "clean"};
	rms = (char **[5]){(char *[3]){whos[0]}, (char *[3]){whos[1]},
	                   (char *[3]){GENCOLORS, GENFONT, SIMPLEXPM},
	                   (char *[3]){COLORS, FONT}, (char *[3]){whos[4]}};
	for (i = 0; i < 5; ++i) {
		if ((cpid = fork()) == 0)
			run(RM, (char *[]){"rm", "-f", rms[i][0], rms[i][1], rms[i][2], NULL},
			    what, whos[i]);
		await(cpid, what, whos[i]);
	}
	free(whos[0]);

	return EXIT_SUCCESS;
}
