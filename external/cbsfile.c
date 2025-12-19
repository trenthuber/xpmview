struct cbsfile {
	char *name, **flags, type;
};

void buildfiles(struct cbsfile *files) {
	char **c, **l, **names;
	struct cbsfile *target;
	size_t i;

	c = cflags;
	l = lflags;

	target = files++;

	for (i = 0; files[i].name; ++i) if (files[i].flags) {
		cflags = files[i].flags;
		compile(files[i].name);
	}

	if (!(names = calloc(i + 1, sizeof*names)))
		err(EXIT_FAILURE, "Memory allocation");
	for (i = 0; files[i].name; ++i) names[i] = files[i].name;

	lflags = target->flags;
	load(target->type, target->name, names);

	free(names);

	cflags = c;
	lflags = l;
}
