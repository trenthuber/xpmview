#ifndef _XPM_MODE_H_
#define _XPM_MODE_H_

typedef enum {
	XPM_MODE_MONO = 0,
	XPM_MODE_SYMBOLIC,
	XPM_MODE_GRAYSCALE_4,
	XPM_MODE_GRAYSCALE,
	XPM_MODE_COLOR,
	NUM_XPM_MODES,
} Xpm_Mode;

Xpm_Mode convert_token_to_mode(const char *token);

#endif // _XPM_MODE_H_
