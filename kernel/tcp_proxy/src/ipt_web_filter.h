/*
	web filter (experimental)
	http(web) request filter moudle
	Copyright (C) 2014 miwifi-network

	Licensed under GNU GPL v2 or later.
*/

#ifndef _IPT_WEB_FILTER_H
#define _IPT_WEB_FILTER_H

//#define __WF_DEBUG__
#ifdef  __WF_DEBUG__
#define LOG     printk
#define LOGHEX(str, len)  {\
    int i = 0;             \
    for (i = 0; i < len; i++)       \
        LOG("%02x ", str[i]);  \
    LOG("\n");  \
}
#define LOGSTR(str, len)  {\
    int i = 0;             \
    for (i = 0; i < len; i++)       \
        LOG("%c", str[i]);  \
    LOG("\n");  \
}
#else
#define LOG(...)    do { } while (0);
#define LOGHEX(...) do { } while (0);
#define LOGSTR(...) do { } while (0);
#endif

typedef enum
{
    IPT_WEB_HTTP,
    IPT_WEB_URI,
    IPT_WEB_HOST,
    IPT_WEB_HORE
} ipt_webfilter_mode_t;

struct ipt_webfilter_info
{
    ipt_webfilter_mode_t mode;
};

#endif
