#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define PROC_DEV_FILE		"/proc/net/dev"
#define IFNAMSIZ 18
struct dev_node {
	char    ifname[IFNAMSIZ];
	uint64_t   rx_bytes;
	uint64_t   rx_packets;
	uint64_t   tx_bytes;
	uint64_t   tx_packets;
};

struct dev_entry {
	char    ifname[IFNAMSIZ];
	long long unsigned int   rx_bytes;
	uint32_t   rx_packets;
	uint32_t   rx_errs;
	uint32_t   rx_drop;
	uint32_t   rx_fifo;
	uint32_t   rx_frame;
	uint32_t   rx_compressed;
	uint32_t   rx_multicast;
	long long unsigned int   tx_bytes;
	uint32_t   tx_packets;
	uint32_t   tx_errs;
	uint32_t   tx_drop;
	uint32_t   tx_fifo;
	uint32_t   tx_frame;
	uint32_t   tx_compressed;
	uint32_t   tx_multicast;
};

#define DPRINTF(format, ...) fprintf(stderr, "%s(%d): " format, __func__, __LINE__, ## __VA_ARGS__)
#define D(format, ...) do { \
		DPRINTF(format, ##__VA_ARGS__); \
	} while (0)


int main(int argc, const char *argv[])
{
	FILE *fp;
	char line[200];
	struct dev_entry dev;
	int num;
	char *p;

	/* Open the PROCps kernel table. */
	if ((fp = fopen(PROC_DEV_FILE, "r")) == NULL) {
		D("fopen %s error\n", PROC_DEV_FILE);
		return;
	}

	if (fgets(line, sizeof(line), fp) != (char *) NULL) {
		/* Read the ARP cache entries. */
		for (; fgets(line, sizeof(line), fp);) {
			num = sscanf(line, "%s %Lu %u %u %u %u %u %u %u "
					"%Lu %u %u %u %u %u %u %u\n",
					dev.ifname, &dev.rx_bytes, &dev.rx_packets, &dev.rx_errs,
					&dev.rx_drop, &dev.rx_fifo, &dev.rx_frame, &dev.rx_compressed, &dev.rx_multicast,
					&dev.tx_bytes, &dev.tx_packets, &dev.tx_errs,
					&dev.tx_drop, &dev.tx_fifo, &dev.tx_frame, &dev.tx_compressed, &dev.tx_multicast
					);
			if (num < 17){
				D("skip %d %s\n", num, line);
				continue;
			}


			D("num:%d ifname:%s rx_bytes:%llu rx_packets:%u tx_bytes:%llu tx_packets:%u\n",  num, dev.ifname, dev.rx_bytes, dev.rx_packets, dev.tx_bytes, dev.tx_packets);
		}
	}
	fclose(fp);

	return 0;
}
