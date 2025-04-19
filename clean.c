#define ROOT
#include "build.h"

#define COLORS SRC "colors.c"
#define FIND "/usr/bin/find"
#define FONT SRC "font.c"
#define GENCOLORS SRC "colors/gencolors"
#define GENFONT SRC "font/genfont"
#define RM "/bin/rm"

#include "external/cbs/cbs.c"

int main(void) {
	char *what, **whos, **regexes, ***rms;
	size_t i;
	pid_t cpid;

	what = "removal";

	// Remove build executables and object files
	whos = (char *[2]){"{*/*build,*/*/*build}", "{*/*.o,*/*/*.o,*/*/*/*.o}"};
	regexes = (char *[2]){".*[^\\.]/build$", ".*\\.o$"};
	for (i = 0; i < 2; ++i) {
		if ((cpid = fork()) == 0)
			run(FIND, (char *[]){"find", ".", "-regex", regexes[i],
			                     "-exec", "rm", "{}", "+", NULL}, what, whos[i]);
		await(cpid, what, whos[i]);
	}

	/* Remove raylib library, application executables,
	 * automatically generated source files, and ourself
	 */
	whos = (char *[4]){extend(RLLIB, RLEXT),
	                   "{" GENCOLORS "," GENFONT "," SIMPLEXPM "}",
	                   "{" COLORS "," FONT "}", "clean"};
	rms = (char **[4]){(char *[3]){whos[0]},
	                   (char *[3]){GENCOLORS, GENFONT, SIMPLEXPM},
	                   (char *[3]){COLORS, FONT}, (char *[3]){whos[3]}};
	for (i = 0; i < 4; ++i) {
		if ((cpid = fork()) == 0)
			run(RM, (char *[]){"rm", "-f", rms[i][0], rms[i][1], rms[i][2], NULL},
			    what, whos[i]);
		await(cpid, what, whos[i]);
	}
	free(whos[0]);

	return EXIT_SUCCESS;
}
