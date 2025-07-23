#ifdef __APPLE__
#define DYEXT ".dylib"
#else
#define DYEXT ".so"
#endif

#define LIST(...) (char *[]){__VA_ARGS__, NULL}

void *allocate(size_t s);
void compile(char *src);
void load(char type, char *target, char **objs);
