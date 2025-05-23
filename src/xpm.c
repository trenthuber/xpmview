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

static char *strnsub(char *str, char *sub, size_t l) {
	size_t subl;

	subl = strlen(sub);
	while (l-- >= subl) if (strncmp(str++, sub, subl) == 0) return str - 1;
	return NULL;
}

static int space(char c) {
	return c == ' ' || c == '\t';
}

static char *arrname(char *p, size_t l) {
	size_t step;
	char *start, *r;

	for (; (step = 5); l = l > step ? l - step : 0, p += step) {
		if (l == 0) return NULL;
		if (space(*p) || *p == '*') step = 1; else
			if (strncmp(p, "const", 5) != 0
			    || (!space(*(p + step)) && *(p + step) != '*'))
				break;
	}

	start = p;
	for (; !space(*p) && *p != '['; ++p, --l) if (l == 0) return NULL;
	l = p - start;
	r = allocate(l + 1);
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

	// RGB
	if (**strp == '#') r = strtol(++*strp, strp, 16); else

		// Color names
		for (; i < numcolors; ++i) {
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

static Image parse(char **data, long *sizep) {
	Image result;
	char *p, *chars, **pp;
	long width, height, ncolors, cpp, l;
	unsigned int *colors, color, *pixels;
	int i, mode, j, k, m;

	// Values
	result = (Image){0};
	p = data[0];
	width = strtol(p, &p, 10);
	height = strtol(p, &p, 10);
	ncolors = strtol(p, &p, 10);
	cpp = strtol(p, &p, 10);
	if (1 + ncolors + height > *sizep) {
		warnx("Actual image height too short");
		return result;
	}

	// Colors
	chars = allocate(ncolors * cpp * sizeof*chars);
	colors = allocate(NUMMODES * ncolors * sizeof*colors);
	for (i = 0; i < ncolors; ++i) {
		p = data[1 + i];
		strncpy(chars + i * cpp, p, cpp);
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

	// Pixels
	pixels = allocate(NUMMODES * height * width * sizeof*pixels);
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
	if (j != width || l != 0) {
		warnx("Actual image width too narrow");
		goto free;
	}

	result = (Image){
		.data = pixels,
		.width = width,
		.height = height,
		.mipmaps = 1,
		.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
	};

free:
	free(chars);
	free(colors);

	return result;
}

static Image process(char *xpm) {
	Image result;
	int xpmfd, srcfd, e, cpid, status;
	struct stat xstat;
	size_t l, offset;
	char *map, *p, *a, *tmp, **data;
	void *d;
	long *sizep;

	result = (Image){0};
	if ((xpmfd = open(xpm, O_RDONLY)) == -1) {
		warn("Unable to open `%s'", xpm);
		return result;
	}
	if (stat(xpm, &xstat) == -1) {
		warn("Unable to stat `%s'", xpm);
		goto close;
	}
	l = xstat.st_size;
	if ((map = mmap(NULL, l, PROT_READ, MAP_PRIVATE, xpmfd, 0)) == MAP_FAILED) {
		warn("Unable to map `%s' to memory", xpm);
		goto close;
	}

	if ((p = strnsub(map, "char", l)) == NULL) { // Skip "static" keyword
		warnx("`%s' improperly formatted", xpm);
		goto munmap;
	}
	offset = p - map;
	if ((a = arrname(p + 4, l - offset - 4)) == NULL) {
		warnx("`%s' improperly formatted", xpm);
		goto munmap;
	}

	if ((srcfd = open(TMPSRC, O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1) {
		warn("Unable to open `" TMPSRC "'");
		goto munmap;
	}
	e = !writeall(srcfd, p, l - offset)
	    || dprintf(srcfd, "\n\nlong size = sizeof %s / sizeof*%s;\n", a, a) < 0;
	if (close(srcfd) == -1) {
		warn("Unable to close `" TMPSRC "'");
		goto munmap;
	}
	if (e) {
		warn("Unable to write to `" TMPSRC "'");
		goto munmap;
	}

	if ((cpid = fork()) == 0) {
		compile(TMP, NULL);
		load('d', TMP, TMP, NULL);
		exit(EXIT_SUCCESS);
	}
 	if (cpid == -1 || waitpid(cpid, &status, 0) == -1) {
		warn("Unable to create `" TMPLIB "'");
		goto munmap;
	}
	if (!WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) goto munmap;

	if ((d = dlopen(TMPLIB, RTLD_LAZY)) == NULL) {
		warnx("Unable to load `" TMPLIB "': %s", dlerror());
		goto munmap;
	}
	if ((data = (char **)dlsym(d, a)) == NULL) {
		warnx("Unable to load image data from `" TMPLIB "': `%s'", dlerror());
		goto dlclose;
	}
	if ((sizep = (long *)dlsym(d, "size")) == NULL) {
		warnx("Unable to load image size from `" TMPLIB "': `%s'", dlerror());
		goto dlclose;
	}

	result = parse(data, sizep);

dlclose:
	if (dlclose(d)) {
		warnx("Unable to unload `" TMPLIB "': %s", dlerror());
		result.mipmaps = 0;
	}

munmap:
	if (munmap(map, l) == -1) {
		warn("Unable to unmap `%s' from memory", xpm);
		result.mipmaps = 0;
	}

close:
	if (close(xpmfd) == -1) {
		warn("Unable to close `%s'", xpm);
		result.mipmaps = 0;
	}

	if (result.data && !result.mipmaps) {
		free(result.data);
		result = (Image){0};
	}

	return result;
}

Texture gettexture(char *xpm, Image *image, int mode) {
	static unsigned int *base;
	static Texture2D texture;

	if (xpm) {
		*image = process(xpm);
		if (base) free(base);
		base = image->data;
	}

	if (!base) return (Texture2D){0};
	
	image->data = base + mode * image->width * image->height;
	UnloadTexture(texture);
	return texture = LoadTextureFromImage(*image);
}
