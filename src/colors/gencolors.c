#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char **argv) {
	int result, infd, outfd;
	struct stat instat;
	size_t l, i;
	void *map;
	char *p;
	unsigned char r, g, b;

	result = EXIT_FAILURE;
	if (argc != 3)
		errx(EXIT_FAILURE, "Incorrect number of arguments: %s <input.txt> <output.c>",
		     argv[0]);
	if ((infd = open(argv[1], O_RDONLY)) == -1)
		err(EXIT_FAILURE, "Unable to open `%s' for reading", argv[1]);
	if (fstat(infd, &instat) == -1) {
		warn("Unable to stat `%s'", argv[1]);
		goto closein;
	}
	l = instat.st_size;
	if ((p = map = mmap(NULL, l, PROT_READ | PROT_WRITE,
	                    MAP_PRIVATE, infd, 0)) == MAP_FAILED) {
		warn("Unable to map `%s' to memory", argv[1]);
		goto closein;
	}
	if ((outfd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1) {
		warn("Unable to open `%s' for writing", argv[2]);
		goto munmap;
	}

	if (dprintf(outfd, "#include <stddef.h>\n\n"
	            "#include \"colors.h\"\n\n"
	            "struct color colors[] = {\n"
	            "\t{\"None\", 0x00ffffff},\n") == -1) {
		warn("Unable to write to `%s'", argv[2]);
		goto closeout;
	}
	for (i = 1; p < (char *)map + l; ++i) {
		r = strtol(p, &p, 10);
		g = strtol(p, &p, 10);
		b = strtol(p, &p, 10);
		if (dprintf(outfd, "\t{\"%s\", 0x%02x%02x%02x},\n",
		            strsep(&p, "\n") + 2, r, g, b) == -1) {
			warn("Unable to write to `%s'", argv[2]);
			goto closeout;
		}
	}
	if (dprintf(outfd, "};\n\nsize_t numcolors = %zu;\n", i) == -1)
		warn("Unable to write to `%s'", argv[2]);

	result = EXIT_SUCCESS;

closeout:
	if (close(outfd) == -1) {
		warn("Unable to close `%s'", argv[2]);
		result = EXIT_FAILURE;
	}

munmap:
	if (munmap(map, l) == -1) {
		warn("Unable to unmap memory associated with `%s'", argv[1]);
		result = EXIT_FAILURE;
	}

closein:
	if (close(infd) == -1) {
		warn("Unable to close `%s'", argv[1]);
		result = EXIT_FAILURE;
	}

	return result;
}
