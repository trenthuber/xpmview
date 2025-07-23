#ifdef __APPLE__
#define DYEXT ".dylib"
#else
#define DYEXT ".so"
#endif

#define NONE (char *[]){NULL}
#define LIST(...) (char *[]){__VA_ARGS__, NULL}

extern char **cflags, **lflags;

void *allocate(size_t s);
void compile(char *src);
void load(char type, char *target, char **objs);
