#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "utilities.h"

int main(int argc, char **argv) {
	int result, infd, outfd;
	struct stat instat;
	size_t l, i;
	void *map;
	char *p;
	unsigned char r, g, b;

	result = EXIT_FAILURE;
	if (argc != 3) {
		xpmerror("Incorrect number of arguments: %s <input.txt> <output.c>", argv[0]);
		return result;
	}
	if ((infd = open(argv[1], O_RDONLY)) == -1) {
		xpmerror("Unable to open `%s' for reading", argv[1]);
		return result;
	}
	if (fstat(infd, &instat) == -1) {
		xpmerror("Unable to stat `%s'", argv[1]);
		goto closein;
	}
	l = instat.st_size;
	if ((p = map = mmap(NULL, l, PROT_READ | PROT_WRITE,
	                    MAP_PRIVATE, infd, 0)) == MAP_FAILED) {
		xpmerror("Unable to map `%s' to memory", argv[1]);
		goto closein;
	}
	if ((outfd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1) {
		xpmerror("Unable to open `%s' for writing", argv[2]);
		goto munmap;
	}

	result = EXIT_SUCCESS;
	if (dprintf(outfd, "struct color {\n"
	            "\tchar *name;\n"
	            "\tunsigned int value;\n"
	            "};\n\n"
	            "static struct color colors[] = {\n"
	            "\t{\"None\", 0x00ffffff},\n") == -1) {
		xpmerror("Unable to write to `%s'", argv[2]);
		goto closeout;
	}
	for (i = 1; p < (char *)map + l; ++i) {
		r = strtol(p, &p, 10);
		g = strtol(p, &p, 10);
		b = strtol(p, &p, 10);
		if (dprintf(outfd, "\t{\"%s\", 0x%02x%02x%02x},\n",
		            strsep(&p, "\n") + 2, r, g, b) == -1) {
			xpmerror("Unable to write to `%s'", argv[2]);
			goto closeout;
		}
	}
	if (dprintf(outfd, "};\n\nstatic size_t numcolors = %zu;\n", i) == -1)
		xpmerror("Unable to write to `%s'", argv[2]);

closeout:
	if (close(outfd) == -1)
		xpmerror("Unable to close `%s'", argv[2]);

munmap:
	if (munmap(map, l) == -1)
		xpmerror("Unable to unmap memory associated with `%s'", argv[1]);

closein:
	if (close(infd) == -1)
		xpmerror("Unable to close `%s'", argv[1]);

	return result;
}
