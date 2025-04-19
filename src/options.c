#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utilities.h"

static void usage(char *prog, int fd) {
	dprintf(fd, "Usage: %s [-dh] [-f file]\n"
	        "\t-d        Show debug messages\n"
	        "\t-f <file> Use <file> as input\n"
	        "\t-h        Show usage message\n\n"
	        "App key bindings:\n"
	        "\t q        Quit app\n"
	        "\t r        Refresh image\n"
	        "\t s        Save image as PNG\n"
	        "\t m        Mono visual mode\n"
	        "\t 4        4-level grayscale mode\n"
	        "\t g        >4-level grayscale mode\n"
	        "\t c        Color visual mode\n", prog);
}

int options(int argc, char **argv, int *debug, char **xp) {
	int opt, result, dnfd;

	*debug = 0;
	*xp = NULL;
	while ((opt = getopt(argc, argv, "df:h")) != -1) switch (opt) {
	case 'd':
		*debug = 1;
		break;
	case 'f':
		*xp = xpmalloc(FILENAME_MAX);
		strcpy(*xp, optarg);
		break;
	case 'h':
		usage(argv[0], STDOUT_FILENO);
		exit(EXIT_SUCCESS);
	case '?':
	default:
		usage(argv[0], STDERR_FILENO);
		exit(EXIT_FAILURE);
	}

	result = 1;
	if (!*debug) {
		if ((dnfd = open("/dev/null", O_WRONLY)) == -1) {
			xpmerror("Unable to open `/dev/null'; showing debug messages");
			return 0;
		}
		if (close(STDOUT_FILENO) == -1) {
			xpmerror("Unable to close stdout");
			result = 0;
		} else if (dup2(dnfd, STDOUT_FILENO) == -1) {
			xpmerror("Unable to redirect stdout to `/dev/null'");
			exit(EXIT_FAILURE);
		}
		if (close(dnfd) == -1) {
			xpmerror("Unable to close `/dev/null'");
			result = 0;
		}
	}

	return result;
}
