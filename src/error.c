#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>

void xpmerror(char *fmt, ...) {
	va_list args;

	dprintf(STDERR_FILENO, "simplexpm: ");
	va_start(args, fmt);
	vdprintf(STDERR_FILENO, fmt, args);
	va_end(args);
	if (errno) {
		dprintf(STDERR_FILENO, ": %s", strerror(errno));
		errno = 0;
	}
	dprintf(STDERR_FILENO, "\n");
}
