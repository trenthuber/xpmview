enum {
	MODEM,
	MODEG4,
	MODEG,
	MODEC,
	NUMMODES,
	SYMBOLIC,
	DEFAULT = MODEC,
};

Texture gettexture(char *path, Image *image, int mode);
