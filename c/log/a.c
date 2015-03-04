#include <stdio.h>
#include <stdint.h>
#include <syslog.h>
#include <stdarg.h>

enum {
	DEBUG_A = 0,
	DEBUG_B,
	DEBUG_C
};

enum {
	L_CRIT,
	L_ERR,
	L_WARNING,
	L_NOTICE,
	L_INFO,
	L_DEBUG
};

static const int log_class[] = {
	[L_CRIT] = LOG_CRIT,
	[L_ERR] = LOG_ERR,
	[L_WARNING] = LOG_WARNING,
	[L_NOTICE] = LOG_NOTICE,
	[L_INFO] = LOG_INFO,
	[L_DEBUG] = LOG_DEBUG
};

uint32_t debug_mask = 1 << DEBUG_A;
uint32_t log_level = L_DEBUG;
uint8_t use_syslog = 0;

#define DPRINTF(format, ...) fprintf(stderr, "%s:%d:%s(): " format, __FILE__, __LINE__, __func__, ## __VA_ARGS__)
#define D(level, format, ...) do { \
		if (debug_mask & (1 << (DEBUG_ ## level))) \
				DPRINTF(format, ##__VA_ARGS__); \
	} while (0)

static void _log_message(int priority, const char *file, const char *func, int line, const char *format, ...)
{
	char buf[255];
	snprintf(buf, sizeof(buf), "%s:%d:%s(): %s", file, line, func, format);
	va_list vl;

	if (priority > log_level)
		return;

	va_start(vl, format);
	if (use_syslog)
		vsyslog(log_class[priority], buf, vl);
	else
		vfprintf(stderr, buf, vl);
	va_end(vl);
}


#define dlog(format, ...) _log_message(L_DEBUG, __FILE__, __func__, __LINE__, format, ## __VA_ARGS__)
#define elog(format, ...) _log_message(L_ERR, __FILE__, __func__, __LINE__, format, ## __VA_ARGS__)

int main(int argc, const char *argv[])
{
	D(A, "1 hello world!\n");	
	D(B, "2 hello world!\n");	
	dlog("3 hello world!\n");
	return 0;
}
