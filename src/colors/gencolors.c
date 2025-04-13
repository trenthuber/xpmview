#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "error.h"

int main(void) {
	int result, infd, outfd, i;
	struct stat instat;
	size_t len;
	void *map;
	char *p;
	long r, g, b;

	result = 1;
	if ((infd = open("rgb.txt", O_RDONLY)) == -1) {
		xpmerror("Unable to open `rgb.txt' for reading");
		return result;
	}
	if (fstat(infd, &instat) == -1) {
		xpmerror("Unable to stat `rgb.txt'");
		goto closein;
	}
	len = instat.st_size;
	if ((p = map = mmap(NULL, len, PROT_READ | PROT_WRITE,
	                    MAP_PRIVATE, infd, 0)) == MAP_FAILED) {
		xpmerror("Unable to map `rgb.txt' to memory");
		goto closein;
	}
	if ((outfd = open("../colors.c", O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1) {
		xpmerror("Unable to open `../colors.c' for writing");
		goto munmap;
	}

	result = 0;
	if (dprintf(outfd, "struct color {\n"
	            "\tchar *name;\n"
	            "\tunsigned int value;\n"
	            "};\n\n"
	            "static struct color colors[] = {\n"
	            "\t{\"None\", 0x00ffffff},\n") == -1) {
		xpmerror("Unable to write to `../colors.c'");
		goto closeout;
	}
	for (i = 1; p < (char *)map + len; ++i) {
		r = strtol(p, &p, 10);
		g = strtol(p, &p, 10);
		b = strtol(p, &p, 10);
		if (dprintf(outfd, "\t{\"%s\", 0x%02lx%02lx%02lx},\n",
		            strsep(&p, "\n") + 2, r, g, b) == -1) {
			xpmerror("Unable to write to `../colors.c'");
			goto closeout;
		}
	}
	if (dprintf(outfd, "};\n\nstatic size_t numcolors = %d;\n", i) == -1)
		xpmerror("Unable to write to `../colors.c'");

closeout:
	if (close(outfd) == -1)
		xpmerror("Unable to close `../colors.c'");

munmap:
	if (munmap(map, len) == -1)
		xpmerror("Unable to unmap memory associated with `rgb.txt'");

closein:
	if (close(infd) == -1)
		xpmerror("Unable to close `rgb.txt'");

	return result;
}
