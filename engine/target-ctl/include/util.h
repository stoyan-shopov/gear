#define panic(__msg) do { printf("%s:%i, %s: %s\n", __FILE__, __LINE__, __func__, __msg); fflush(stdout); fflush(stderr); while (1); } while (0)

extern int xprintf(int sock_fd, const char * format, ...);

