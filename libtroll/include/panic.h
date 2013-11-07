#define panic(__msg)	do { printf("%s, %s, %i, %s\n", __func__, __FILE__, __LINE__, __msg); fflush(stdout); fflush(stderr); while(1); } while (0)
