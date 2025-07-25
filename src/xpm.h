enum {
	MODEM,
	MODEG4,
	MODEG,
	MODEC,
	NUMMODES,
	SYMBOLIC,
	DEFAULT = MODEC,
};

extern Image image;

Texture2D *reloadtexture(char *xpm, int mode);
