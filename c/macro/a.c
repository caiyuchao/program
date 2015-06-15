#define _add_env(a,b) do { \
	snprintf(buff, TRAFFICD_KV_SIZELIMIT, _{a}"=%s", b); \
	if(uevent_add_env(event, buff, argv)) \
		goto child_out; \
	} while (0)

#define _add(a) add.a
_add_env(EVENT, 1 ?
		"had associated successfully" : "had disassociated");

_add(b)
