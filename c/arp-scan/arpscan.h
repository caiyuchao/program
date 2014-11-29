#define err_msg(format, ...) do{ \
		fprintf(stderr, "%s(%d): " format, __func__, __LINE__, ## __VA_ARGS__); \
		exit(1); \
	}