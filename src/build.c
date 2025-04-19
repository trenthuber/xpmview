#define ROOT "../"
#include "../build.h"

#include "../external/cbs/cbs.c"

int main(void) {
	build(NULL);

	compile("utilities", NULL);

	build("colors/");
	build("font/");

	cflags = (char *[]){CFRAYLIB, NULL};
	compile("main", "utilities", "options", RLHDR, "xpm", "font.c", NULL);
	cflags = NULL;
	compile("options", "utilities", NULL);
	cflags = (char *[]){CFRAYLIB, CFCBS, NULL};
	compile("xpm", "utilities", RLHDR, "xpm", CBS, "colors.c", NULL);

	lflags = (char *[]){LFRAYLIB, NULL};
	load('x', SIMPLEXPM, "utilities", "main", "options", "xpm", RLLIB, NULL);

	return EXIT_SUCCESS;
}
