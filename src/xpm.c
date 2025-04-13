#include <dlfcn.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>
#include <sys/mman.h>

#include "error.h"
#include "raylib.h"
#include "xpm.h"

#include "cbs.c"
#include "colors.c"

static int space(char c) {
	return c == ' ' || c == '\t';
}

static void *zalloc(size_t size) {
	void *result;

	if ((result  = calloc(1, size))) return result;

	xpmerror("Out of memory");
	exit(EXIT_FAILURE);
}

static char *arrname(char *p, size_t len) {
	int step;
	char *start, *result;

	for (; (step = 5); len = len > step ? len - step : 0, p += step) {
		if (len == 0) return NULL;
		if (space(*p) || *p == '*') step = 1;
		else if (strncmp(p, "const", step) != 0
		         || (!space(*(p + step)) && *(p + step) != '*'))
			break;
	}

	start = p;
	for (; !space(*p) && *p != '['; ++p, --len) if (len == 0) return NULL;
	len = p - start;
	result = zalloc(len + 1);
	strncpy(result, start, len);
	result[len] = '\0';

	return result;
}

static int writeall(int fd, char *buf, size_t size) {
	ssize_t n;

	while (size > 0) {
		if ((n = write(fd, buf, size)) == -1) return 0;
		buf += n;
		size -= n;
	}
	if (size < 0) return 0;
	return 1;
}

static int key2mode(char **strp) {
	int result;

	switch (*(*strp)++) {
	case 'm':
		result = MODEM;
		break;
	case 's':
		result = SYMBOLIC;
		break;
	case 'g':
		if (**strp == '4') {
			++*strp;
			result = MODEG4;
			break;
		}
		result = MODEG;
		break;
	case 'c':
		result = MODEC;
		break;
	default:
		xpmerror("Unknown key `%c'", *(*strp - 1));
		result = NUMMODES;
	}

	while (space(**strp)) ++*strp;

	return result;
}

static int lendian(void) {
	int i;

	i = 1;
	return *(char *)&i;
}

static unsigned int str2color(char **strp) {
	unsigned int i, result, value;
	size_t len;
	char *name;

	len = i = 0;

	// RGB
	if (**strp == '#') result = strtol(++*strp, strp, 16);

	// Color names
	else for (; i < numcolors; ++i) {
		name = colors[i].name;
		value = colors[i].value;
		len = strlen(name);
		if (strncmp(name, *strp, len) == 0
		    && (i == 0 || *(*strp + len) == '\0' || space(*(*strp + len)))) {
			result = value;
			break;
		}
	}
	if (i == numcolors) {
		xpmerror("`%s' is not a valid color name", *strp);
		return 0;
	}

	if (result > 0xffffff || result < 0) {
		xpmerror("`0x%06x' is not a valid RGB color", result);
		return 0;
	}
	if (lendian())
		result = (result >> 16 & 0xff)
		         | (result & 0xff00)
		         | (result & 0xff) << 16;
	if (strncmp(name, "None", 4) != 0)
		result |= 0xff000000;

	*strp += len;
	while (space(**strp)) ++*strp;

	return result;
}

static Image parse(char **data, long *sizep) {
	Image result;
	char *p, *chars, **pp;
	long width, height, ncolors, cpp;
	unsigned int *colors, color, *pixels;
	int i, mode, j, k, m;
	size_t len;

	// Values
	result = (Image){0};
	p = data[0];
	width = strtol(p, &p, 10);
	height = strtol(p, &p, 10);
	ncolors = strtol(p, &p, 10);
	cpp = strtol(p, &p, 10);
	if (1 + ncolors + height > *sizep) {
		xpmerror("Actual image height too short");
		return result;
	}

	// Colors
	chars = zalloc(ncolors * cpp * sizeof*chars);
	colors = zalloc(NUMMODES * ncolors * sizeof*colors);
	for (i = 0; i < ncolors; ++i) {
		p = data[1 + i];
		strncpy(&chars[i * cpp], p, cpp);
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
	pixels = zalloc(NUMMODES * height * width * sizeof*pixels);
	j = width;
	len = 0;
	for (i = 0, pp = &data[1 + ncolors];
	     i < height && j == width && len == 0;
	     ++i, ++pp)
		for (j = 0, p = *pp, len = strlen(p);
		     j < width && len > 0;
		     ++j, p += cpp, len = len > cpp ? len - cpp : 0)
			for (k = 0; k < ncolors; ++k)
				if (strncmp(p, &chars[k * cpp], cpp) == 0)
					for (m = 0; m < NUMMODES; ++m)
						pixels[m * width * height + i * width + j] = colors[m * ncolors + k];
	if (j != width || len > 0) {
		xpmerror("Actual image width too narrow");
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
	size_t len, offset;
	char *map, *p, *a, **data;
	void *d;
	long *sizep;

	errno = 0;
	result = (Image){0};
	if ((xpmfd = open(xpm, O_RDONLY)) == -1) {
		xpmerror("Unable to open `%s'", xpm);
		return result;
	}
	if (stat(xpm, &xstat) == -1) {
		xpmerror("Unable to stat `%s'", xpm);
		goto close;
	}
	len = xstat.st_size;
	if ((map = mmap(NULL, len, PROT_READ, MAP_PRIVATE, xpmfd, 0)) == MAP_FAILED) {
		xpmerror("Unable to map `%s' to memory", xpm);
		goto close;
	}

	if ((p = strnstr(map, "char", len)) == NULL) { // Skip "static" keyword
		xpmerror("`%s' improperly formatted", xpm);
		goto munmap;
	}
	offset = p - map;
	if ((a = arrname(p + 4, len - offset - 4)) == NULL) {
		xpmerror("`%s' improperly formatted", xpm);
		goto munmap;
	}

	if ((srcfd = open("/tmp/xpm.c", O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1) {
		xpmerror("Unable to open `/tmp/xpm.c'");
		goto munmap;
	}
	e = !writeall(srcfd, p, len - offset)
	    || dprintf(srcfd, "\n\nlong size = sizeof %s / sizeof*%s;\n", a, a) < 0;
	if (close(srcfd) == -1) {
		xpmerror("Unable to close `/tmp/xpm.c'");
		goto munmap;
	}
	if (e) {
		xpmerror("Unable to write to `/tmp/xpm.c'");
		goto munmap;
	}

	if ((cpid = fork()) == 0) {
		compile("/tmp/xpm", NULL);
		load('d', "/tmp/xpm", "/tmp/xpm", NULL);
		exit(EXIT_SUCCESS);
	}
 	if (cpid == -1 || waitpid(cpid, &status, 0) == -1
	    || !WIFEXITED(status) || WEXITSTATUS(status) != EXIT_SUCCESS) {
		xpmerror("Unable to create `/tmp/libxpm.dylib'");
		goto munmap;
	}

	if ((d = dlopen("/tmp/libxpm.dylib", 0)) == NULL) {
		xpmerror("Unable to load `/tmp/libxpm.dylib': %s", dlerror());
		goto munmap;
	}
	if ((data = (char **)dlsym(d, a)) == NULL) {
		xpmerror("Unable to load image data from `/tmp/libxpm.dylib': `%s'", dlerror());
		goto dlclose;
	}
	if ((sizep = (long *)dlsym(d, "size")) == NULL) {
		xpmerror("Unable to load image length from `/tmp/libxpm.dylib': `%s'",
		      dlerror());
		goto dlclose;
	}

	result = parse(data, sizep);

dlclose:
	if (dlclose(d))
		xpmerror("Unable to unload `/tmp/libxpm.dylib': %s", dlerror());

munmap:
	if (munmap(map, len) == -1)
		xpmerror("Unable to unmap `%s' from memory", xpm);

close:
	if (close(xpmfd) == -1)
		xpmerror("Unable to close `%s'", xpm);

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
