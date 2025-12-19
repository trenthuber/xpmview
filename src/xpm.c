#include <dlfcn.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "cbs.h"
#include "colors.h"
#include "raylib.h"
#include "xpm.h"

#define TMP "/tmp/xpm"
#define TMPSRC TMP ".c"
#define TMPLIB "/tmp/libxpm" DYEXT

Image image;

static char *strnsub(char *str, char *sub, size_t l) {
	size_t subl;

	subl = strlen(sub);
	while (l-- >= subl) if (strncmp(str++, sub, subl) == 0) return str - 1;
	return NULL;
}

static int space(char c) {
	return c == ' ' || c == '\t';
}

static char *getname(char *p, size_t l) {
	size_t step;
	char *start, *r;

	for (; (step = 5); l = l > step ? l - step : 0, p += step) {
		if (l == 0) return NULL;
		if (space(*p) || *p == '*') step = 1;
		else if (strncmp(p, "const", 5) != 0
		         || (!space(*(p + step)) && *(p + step) != '*'))
			break;
	}

	for (start = p; !space(*p) && *p != '['; ++p, --l) if (l == 0) return NULL;
	if (!(r = calloc((l = p - start) + 1, sizeof*r)))
		err(EXIT_FAILURE, "Memory allocation");
	strncpy(r, start, l);

	return r;
}

static int writeall(int fd, char *buf, size_t n) {
	ssize_t w;

	while (n > 0) {
		if ((w = write(fd, buf, n)) == -1) return 0;
		buf += w;
		n -= w;
	}
	return 1;
}

static int key2mode(char **strp) {
	int r;

	switch (*(*strp)++) {
	case 'm':
		r = MODEM;
		break;
	case 'g':
		if (**strp == '4') {
			++*strp;
			r = MODEG4;
			break;
		}
		r = MODEG;
		break;
	case 'c':
		r = MODEC;
		break;
	default:
		warnx("Unknown key `%c'", *(*strp - 1));
		r = NUMMODES;
		break;
	case 's':
		r = SYMBOLIC;
	}

	while (space(**strp)) ++*strp;

	return r;
}

static char lendian(void) {
	size_t i;

	i = 1;
	return *(char *)&i;
}

static unsigned int str2color(char **strp) {
	size_t i, l;
	unsigned int r;
	char *name;

	l = i = 0;

	/* RGB */
	if (**strp == '#') r = strtol(++*strp, strp, 16);

	/* Color names */
	else for (; i < numcolors; ++i) {
		name = colors[i].name;
		l = strlen(name);
		if (strncmp(name, *strp, l) == 0
		    && (i == 0 || *(*strp + l) == '\0' || space(*(*strp + l)))) {
			r = colors[i].value;
			break;
		}
	}
	if (i == numcolors) {
		warnx("`%s' is not a valid color name", *strp);
		return 0;
	}

	if (r > 0xffffff) {
		warnx("`0x%06x' is not a valid RGB color", r);
		return 0;
	}
	if (lendian()) r = (r >> 16 & 0xff) | (r & 0xff00) | (r & 0xff) << 16;
	if (strcmp(name, "None") != 0) r |= 0xff000000;

	*strp += l;
	while (space(**strp)) ++*strp;

	return r;
}

static void parse(char **data, long *sizep) {
	char *p, *chars, **pp;
	long width, height, ncolors, cpp, l;
	unsigned int *colors, color, *pixels;
	int i, mode, j, k, m;

	/* Values */
	width = strtol(p = data[0], &p, 10);
	height = strtol(p, &p, 10);
	ncolors = strtol(p, &p, 10);
	cpp = strtol(p, &p, 10);
	if (1 + ncolors + height > *sizep) {
		warnx("Actual image height too short");
		return;
	}

	/* Colors */
	if (!(chars = calloc(ncolors * cpp, sizeof*chars))
	    || !(colors = calloc(NUMMODES * ncolors, sizeof*colors)))
		err(EXIT_FAILURE, "Memory allocation");
	for (i = 0; i < ncolors; ++i) {
		strncpy(chars + i * cpp, p = data[1 + i], cpp);
		p += cpp;
		while (space(*p)) ++p;
		while (*p) switch ((mode = key2mode(&p))) {
		case SYMBOLIC:
			while (*p && !space(*p)) ++p;
			while (space(*p)) ++p;
			continue;
		default:
			if ((color = str2color(&p)) == 0)
			case NUMMODES:
				goto free;
			colors[mode * ncolors + i] = color;
		}
	}

	/* Pixels */
	if (!(pixels = calloc(NUMMODES * height * width, sizeof*pixels)))
		err(EXIT_FAILURE, "Memory allocation");
	j = width;
	l = 0;
	for (i = 0, pp = &data[1 + ncolors];
	     i < height && j == width && l == 0;
	     ++i, ++pp)
		for (j = 0, p = *pp, l = strlen(p);
		     j < width && l > 0;
		     ++j, p += cpp, l -= cpp)
			for (k = 0; k < ncolors; ++k)
				if (strncmp(p, chars + k * cpp, cpp) == 0)
					for (m = 0; m < NUMMODES; ++m)
						pixels[m * width * height + i * width + j] = colors[m * ncolors + k];
	if (j != width || l != 0) warnx("Actual image width too narrow");
	else image = (Image){.data = pixels, .width = width, .height = height,
	                     .mipmaps = 1,
	                     .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};

free:
	free(chars);
	free(colors);
}

static void reloadimage(char *xpm) {
	int xpmfd, srcfd, e, cpid, status;
	struct stat xstat;
	size_t l, offset;
	char *map, *p, *name, *src, *lib, **data;
	void *d;
	long *sizep;

	image = (Image){0};
	if ((xpmfd = open(xpm, O_RDONLY)) == -1) {
		warn("Unable to open `%s'", xpm);
		return;
	}
	if (stat(xpm, &xstat) == -1) {
		warn("Unable to stat `%s'", xpm);
		goto close;
	}
	if ((map = mmap(NULL, l = xstat.st_size, PROT_READ, MAP_PRIVATE, xpmfd, 0))
	    == MAP_FAILED) {
		warn("Unable to map `%s' to memory", xpm);
		goto close;
	}

	if ((p = strnsub(map, "char", l)) == NULL) { /* Skip "static" keyword */
		warnx("`%s' improperly formatted", xpm);
		goto munmap;
	}
	if ((name = getname(p + 4, l - (offset = p - map) - 4)) == NULL) {
		warnx("`%s' improperly formatted", xpm);
		goto munmap;
	}

	src = "/tmp/xpm.c";
	if ((srcfd = open(src, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1) {
		warn("Unable to open `%s'", src);
		goto munmap;
	}
	e = !writeall(srcfd, p, l - offset)
	    || dprintf(srcfd, "\n\nlong size = sizeof %s / sizeof*%s;\n", name, name) < 0;
	if (close(srcfd) == -1) {
		warn("Unable to close `%s'", src);
		goto munmap;
	}
	if (e) {
		warn("Unable to write to `%s'", src);
		goto munmap;
	}

	lib = "/tmp/libxpm" DYEXT;
	if ((cpid = fork()) == -1) {
		warn("Unable to fork process to create `%s'", lib);
		goto munmap;
	}
	if (!cpid) {
		cflags = NONE;
		compile("/tmp/xpm");

		lflags = NONE;
		load('d', "/tmp/xpm", LIST("/tmp/xpm"));

		/* I sadly didn't leave a comment when I initially wrote this part, but from
		 * what I remember, we use `_exit()' here because Raylib registers cleanup
		 * functions with `atexit()' that we don't want to call from the child
		 * process. I think calling the usual `exit()' causes the child to crash. */
		_exit(EXIT_SUCCESS);
	}
 	if (cpid == -1 || waitpid(cpid, &status, 0) == -1) {
		warn("Creation of `%s' terminated unexpectedly", lib);
		goto munmap;
	}
	if (WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS
	    || WIFSIGNALED(status)) {
		warnx("Creation of `%s' was unsuccessful", lib);
		goto munmap;
	}

	if ((d = dlopen(lib, RTLD_LAZY)) == NULL) {
		warnx("Unable to load `%s': %s", lib, dlerror());
		goto munmap;
	}
	if ((data = (char **)dlsym(d, name)) == NULL) {
		warnx("Unable to load image data from `%s': `%s'", lib, dlerror());
		goto dlclose;
	}
	if ((sizep = (long *)dlsym(d, "size")) == NULL) {
		warnx("Unable to load image size from `%s': `%s'", lib, dlerror());
		goto dlclose;
	}

	parse(data, sizep);

dlclose:
	if (dlclose(d)) {
		warnx("Unable to unload `%s': %s", lib, dlerror());
		image.mipmaps = 0;
	}

munmap:
	if (munmap(map, l) == -1) {
		warn("Unable to unmap `%s' from memory", xpm);
		image.mipmaps = 0;
	}

close:
	if (close(xpmfd) == -1) {
		warn("Unable to close `%s'", xpm);
		image.mipmaps = 0;
	}

	if (image.data && !image.mipmaps) {
		free(image.data);
		image = (Image){0};
	}
}

Texture2D *reloadtexture(char *xpm, int mode) {
	static unsigned int *base;
	static Texture2D texture;

	if (xpm) {
		reloadimage(xpm);
		if (base) free(base);
		base = image.data;
	}

	if (!base) return NULL;
	
	image.data = base + mode * image.width * image.height;

	UnloadTexture(texture);
	texture = LoadTextureFromImage(image);

	return &texture;
}
