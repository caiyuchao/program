/* 
 * yubo@yubo.org
 * 2015-05-15
 */
#ifndef _DEBUG_H
#define _DEBUG_H
#include <sys/syslog.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>

#define	L_EMERG    LOG_EMERG	/* system is unusable */
#define	L_ALERT    LOG_ALERT	/* action must be taken immediately */
#define	L_CRIT     LOG_CRIT		/* critical conditions */
#define	L_ERR      LOG_ERR		/* error conditions */
#define	L_WARNING  LOG_WARNING	/* warning conditions */
#define	L_NOTICE   LOG_NOTICE	/* normal but significant condition */
#define	L_INFO     LOG_INFO		/* informational */
#define	L_DEBUG    LOG_DEBUG	/* debug-level messages */

static const char *LOG_L[] = {
	[L_EMERG]   = "EMERG",
	[L_ALERT]   = "ALERT",
	[L_CRIT]    = "CRIT",
	[L_ERR]     = "ERR",
	[L_WARNING] = "WARNING",
	[L_NOTICE]  = "NOTICE",
	[L_INFO]    = "INFO" ,
	[L_DEBUG]   = "DEBUG" 
}; 

static inline int __vlog(__attribute__((unused)) uint32_t level,
       const char *format, va_list ap)
{     
    int ret;
 
#ifdef USE_SYSLOG
		ret = vsyslog(level, format, ap);
#else
		ret = vfprintf(stderr, format, ap);
    	fflush(stderr);
#endif
    return ret;
}
      
static inline int __log(uint32_t level, const char *format, ...)
{   
    va_list ap;
    int ret;
      
    va_start(ap, format);
    ret = __vlog(level, format, ap);
    va_end(ap);
    return ret;
}

#ifdef _DEBUG
static inline void log_meesage(int level, const char *func, int line, const char*format, ...){
	if(_DEBUG >= level){
		char buf[512];
		va_list vl;

		snprintf(buf, sizeof(buf), "%-8s%s(%d): %s", LOG_L[level], func, line, format);
		va_start(vl, format);
#ifdef USE_SYSLOG
		vsyslog(level, buf, vl);
#else
		vfprintf(stderr, buf, vl);
#endif
		va_end(vl); 
	}
}

#define _DPRINT(level, format, ...) log_meesage(LOG_ ## level, __func__, __LINE__, format, ## __VA_ARGS__)
#define dlog(format, ...) _DPRINT(DEBUG, format, ## __VA_ARGS__)
#define elog(format, ...) _DPRINT(ERR, format, ## __VA_ARGS__)
#else
#define _DPRINT(level, format, ...) 
#define dlog(level, format, ...) 
#endif

#define p_log(level, format, ...) __log(LOG_ ## level, format, ## __VA_ARGS__)
#define p_vlog(level, format, ap) __vlog(LOG_ ## level, format, ap)

void dump_stack(void);


/**
 * Dump the registers of the calling core to the console.
 *
 * Note: Not implemented in a userapp environment; use gdb instead.
 */
void dump_registers(void);

#define panic(...) panic_(__func__, __VA_ARGS__, "dummy")
#define panic_(func, format, ...) __panic(func, format "%.0s", __VA_ARGS__)

#define	VERIFY(exp)	do {                                                  \
	if (!(exp))                                                           \
		panic("line %d\tassert \"" #exp "\" failed\n", __LINE__); \
} while (0)

void __panic(const char *funcname , const char *format, ...)
#ifdef __GNUC__
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 2))
	__attribute__((cold))
#endif
#endif
	__attribute__((noreturn))
	__attribute__((format(printf, 2, 3)));

void die(int exit_code, const char *format, ...);

#endif
