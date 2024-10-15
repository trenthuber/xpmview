#include <string.h>

#include "xpm_mode.h"
#include "utils.h"

Xpm_Mode convert_token_to_mode(const char *token) {
	if (strcmp(token, "m") == 0) return XPM_MODE_MONO;
	if (strcmp(token, "s") == 0) return XPM_MODE_SYMBOLIC;
	if (strcmp(token, "g4") == 0) return XPM_MODE_GRAYSCALE_4;
	if (strcmp(token, "g") == 0) return XPM_MODE_GRAYSCALE;
	if (strcmp(token, "c") == 0) return XPM_MODE_COLOR;
	SIMPLE_XPM_ERROR("\"%s\" is not a valid color mode", token);
}
