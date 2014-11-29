#include "arpscan.h"


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <inttypes.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/time.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pcap.h>
#include <sys/ioctl.h>


/*
 *	Link layer handle structure for packet socket.
 *	This is typedef'ed as link_t.
 */
typedef struct link_handle {
   int fd;		/* Socket file descriptor */
   struct ifreq ifr;
   struct sockaddr_ll sll;
} link_t;


static link_t *
link_open(const char *device) {
   link_t *handle;

   handle = Malloc(sizeof(*handle));
   memset(handle, '\0', sizeof(*handle));
   if ((handle->fd = socket(PF_PACKET, SOCK_RAW, 0)) < 0) {
      free(handle);
      return NULL;
   }
   strlcpy(handle->ifr.ifr_name, device, sizeof(handle->ifr.ifr_name));
   if ((ioctl(handle->fd, SIOCGIFINDEX, &(handle->ifr))) != 0)
      err_sys("ioctl");
   handle->sll.sll_family = PF_PACKET;
   handle->sll.sll_ifindex = handle->ifr.ifr_ifindex;
   handle->sll.sll_halen = ETH_ALEN;

   return handle;
}

/*
 *	link_close -- Close the link
 *
 *	Inputs:
 *
 *	handle		The handle for the link interface
 *
 *	Returns:
 *
 *	None
 */
static void
link_close(link_t *handle) {
   if (handle != NULL) {
      if (handle->fd != 0)
         close(handle->fd);
      free(handle);
   }
}

/*
 *      get_hardware_address    -- Get the Ethernet MAC address associated
 *                                 with the given device.
 *      Inputs:
 *
 *      if_name		The name of the network interface
 *      hw_address	(output) the Ethernet MAC address
 *
 *      Returns:
 *
 *      None
 */
void
get_hardware_address(const char *if_name, unsigned char hw_address[]) {
   link_t *handle;

   handle = link_open(if_name);

/* Obtain hardware address for specified interface */
   if ((ioctl(handle->fd, SIOCGIFHWADDR, &(handle->ifr))) != 0)
      err_sys("ioctl");

   link_close(handle);

   memcpy(hw_address, handle->ifr.ifr_ifru.ifru_hwaddr.sa_data, ETH_ALEN);
}

/* Global variables */
static host_entry *helist = NULL;	/* Array of host entries */
static host_entry **helistptr;		/* Array of pointers to host entries */
static host_entry **cursor;		/* Pointer to current host entry ptr */
static unsigned num_hosts = 0;		/* Number of entries in the list */
static unsigned responders = 0;		/* Number of hosts which responded */
static unsigned live_count;		/* Number of entries awaiting reply */
static int verbose=0;			/* Verbose level */
static int debug = 0;			/* Debug flag */
static char filename[MAXLINE];		/* Target list file name */
static int filename_flag=0;		/* Set if using target list file */
static int random_flag=0;		/* Randomise the list */
static int numeric_flag=0;		/* IP addresses only */
static unsigned interval=0;		/* Desired interval between packets */
static unsigned bandwidth=DEFAULT_BANDWIDTH; /* Bandwidth in bits per sec */
static unsigned retry = DEFAULT_RETRY;	/* Number of retries */
static unsigned timeout = DEFAULT_TIMEOUT; /* Per-host timeout */
static float backoff_factor = DEFAULT_BACKOFF_FACTOR;	/* Backoff factor */
static int snaplen = SNAPLEN;		/* Pcap snap length */
static char *if_name=NULL;		/* Interface name, e.g. "eth0" */
static int quiet_flag=0;		/* Don't decode the packet */
static int ignore_dups=0;		/* Don't display duplicate packets */
static uint32_t arp_spa;		/* Source IP address */
static int arp_spa_flag=0;		/* Source IP address specified */
static int arp_spa_is_tpa=0;		/* Source IP is dest IP */
static unsigned char arp_sha[ETH_ALEN];	/* Source Ethernet MAC Address */
static int arp_sha_flag=0;		/* Source MAC address specified */
static char ouifilename[MAXLINE];	/* OUI filename */
static char iabfilename[MAXLINE];	/* IAB filename */
static char macfilename[MAXLINE];	/* MAC filename */
static char pcap_savefile[MAXLINE];	/* pcap savefile filename */
static int arp_op=DEFAULT_ARP_OP;	/* ARP Operation code */
static int arp_hrd=DEFAULT_ARP_HRD;	/* ARP hardware type */
static int arp_pro=DEFAULT_ARP_PRO;	/* ARP protocol */
static int arp_hln=DEFAULT_ARP_HLN;	/* Hardware address length */
static int arp_pln=DEFAULT_ARP_PLN;	/* Protocol address length */
static int eth_pro=DEFAULT_ETH_PRO;	/* Ethernet protocol type */
static unsigned char arp_tha[6] = {0, 0, 0, 0, 0, 0};
static unsigned char target_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
static unsigned char source_mac[6];
static int source_mac_flag = 0;
static unsigned char *padding=NULL;
static size_t padding_len=0;
static struct hash_control *hash_table;
static int localnet_flag=0;		/* Scan local network */
static int llc_flag=0;			/* Use 802.2 LLC with SNAP */
static int ieee_8021q_vlan=-1;		/* Use 802.1Q VLAN tagging if >= 0 */
static int pkt_write_file_flag=0;	/* Write packet to file flag */
static int pkt_read_file_flag=0;	/* Read packet from file flag */
static char pkt_filename[MAXLINE];	/* Read/Write packet to file filename */
static int write_pkt_to_file=0;		/* Write packet to file for debugging */
static int rtt_flag=0;			/* Display round-trip time */
static pcap_dumper_t *pcap_dump_handle = NULL;	/* pcap savefile handle */


int main(int argc, char const *argv[])
{
   struct timeval now;
   struct timeval diff;         /* Difference between two timevals */
   int select_timeout;          /* Select timeout */
   ARP_UINT64 loop_timediff;    /* Time since last packet sent in us */
   ARP_UINT64 host_timediff; /* Time since last pkt sent to this host (us) */
   struct timeval last_packet_time;     /* Time last packet was sent */
   int req_interval;		/* Requested per-packet interval */
   int cum_err=0;               /* Cumulative timing error */
   struct timeval start_time;   /* Program start time */
   struct timeval end_time;     /* Program end time */
   struct timeval elapsed_time; /* Elapsed time as timeval */
   double elapsed_seconds;      /* Elapsed time in seconds */
   int reset_cum_err;
   int pass_no = 0;
   int first_timeout=1;
   unsigned i;
   char errbuf[PCAP_ERRBUF_SIZE];
   struct bpf_program filter;
   char *filter_string;
   bpf_u_int32 netmask;
   bpf_u_int32 localnet;
   int datalink;
   int get_addr_status = 0;
   int pcap_fd;			/* Pcap file descriptor */
   unsigned char interface_mac[ETH_ALEN];
   pcap_t *pcap_handle;		/* pcap handle */
   struct in_addr source_ip_address;
/*
 *      Initialise file names to the empty string.
 */
   ouifilename[0] = '\0';
   iabfilename[0] = '\0';
   macfilename[0] = '\0';
   pcap_savefile[0] = '\0';


   char *source_ipa = "192.168.31.27";


  get_hardware_address(if_name, interface_mac);
  if (interface_mac[0]==0 && interface_mac[1]==0 &&
      interface_mac[2]==0 && interface_mac[3]==0 &&
      interface_mac[4]==0 && interface_mac[5]==0) {
     err_msg("ERROR: Could not obtain MAC address for interface %s",
             if_name);
  }

  memcpy(source_mac, interface_mac, ETH_ALEN);
	memcpy(arp_sha, interface_mac, ETH_ALEN);



   if ((inet_pton(AF_INET, source_ipa, &source_ip_address)) <= 0)
      err_msg("inet_pton failed for %s", source_ipa);

   memcpy(&arp_spa, &(source_ip_address.s_addr), sizeof(arp_spa));


      if (!(pcap_handle = pcap_open_live(if_name, snaplen, PROMISC, TO_MS,
          errbuf)))
         err_msg("pcap_open_live: %s", errbuf);


      if ((datalink=pcap_datalink(pcap_handle)) < 0)
         err_msg("pcap_datalink: %s", pcap_geterr(pcap_handle));
      printf("Interface: %s, datalink type: %s (%s)\n",
             pkt_read_file_flag ? "savefile" : if_name,
             pcap_datalink_val_to_name(datalink),
             pcap_datalink_val_to_description(datalink));
      if (datalink != DLT_EN10MB) {
         warn_msg("WARNING: Unsupported datalink type");
      }



      if ((pcap_fd=pcap_get_selectable_fd(pcap_handle)) < 0)
         err_msg("pcap_fileno: %s", pcap_geterr(pcap_handle));
      if ((pcap_setnonblock(pcap_handle, 1, errbuf)) < 0)
         err_msg("pcap_setnonblock: %s", errbuf);

      if (pcap_lookupnet(if_name, &localnet, &netmask, errbuf) < 0) {
         memset(&localnet, '\0', sizeof(localnet));
         memset(&netmask, '\0', sizeof(netmask));
         if (localnet_flag) {
            warn_msg("ERROR: Could not obtain interface IP address and netmask");
            err_msg("ERROR: pcap_lookupnet: %s", errbuf);
         }
      }


/*
 *	The filter string selects packets addressed to our interface address
 *	that are Ethernet-II ARP packets, 802.3 LLC/SNAP ARP packets,
 *	802.1Q tagged ARP packets or 802.1Q tagged 802.3 LLC/SNAP ARP packets.
 */
      filter_string=make_message("ether dst %.2x:%.2x:%.2x:%.2x:%.2x:%.2x and "
                                 "(arp or (ether[14:4]=0xaaaa0300 and "
                                 "ether[20:2]=0x0806) or (ether[12:2]=0x8100 "
                                 "and ether[16:2]=0x0806) or "
                                 "(ether[12:2]=0x8100 and "
                                 "ether[18:4]=0xaaaa0300 and "
                                 "ether[24:2]=0x0806))",
                                 interface_mac[0], interface_mac[1],
                                 interface_mac[2], interface_mac[3],
                                 interface_mac[4], interface_mac[5]);

      if ((pcap_compile(pcap_handle, &filter, filter_string, OPTIMISE,
           netmask)) < 0)
         err_msg("pcap_compile: %s", pcap_geterr(pcap_handle));
      free(filter_string);

      if ((pcap_setfilter(pcap_handle, &filter)) < 0)
         err_msg("pcap_setfilter: %s", pcap_geterr(pcap_handle));

// timeout = 500;

		argv=&argv[optind];
		while (*argv) {
			add_host_pattern(*argv, timeout);
			argv++;
		}


	if (!num_hosts)
		err_msg("ERROR: No hosts to process.");


	/*
	 *      Create and initialise array of pointers to host entries.
	 */
	helistptr = Malloc(num_hosts * sizeof(host_entry *));
	for (i=0; i<num_hosts; i++)
		helistptr[i] = &helist[i];



	/*
	 *      Set current host pointer (cursor) to start of list, zero
	 *      last packet sent time, and set last receive time to now.
	 */
	live_count = num_hosts;
	cursor = helistptr;
	last_packet_time.tv_sec=0;
	last_packet_time.tv_usec=0;

	/*
	 *      Calculate the required interval to achieve the required outgoing
	 *      bandwidth unless the interval was manually specified with --interval.
	 */
	if (!interval) {
		size_t packet_out_len;

		packet_out_len=send_packet(NULL, NULL, NULL); /* Get packet data size */
		if (packet_out_len < MINIMUM_FRAME_SIZE)
			packet_out_len = MINIMUM_FRAME_SIZE;   /* Adjust to minimum size */
		packet_out_len += PACKET_OVERHEAD;	/* Add layer 2 overhead */
		interval = ((ARP_UINT64)packet_out_len * 8 * 1000000) / bandwidth;
		if (verbose > 1) {
			warn_msg("DEBUG: pkt len=%u bytes, bandwidth=%u bps, interval=%u us",
					packet_out_len, bandwidth, interval);
		}
	}

	/*
	 *      Display initial message.
	 */
	printf("Starting %s with %u hosts (http://www.nta-monitor.com/tools/arp-scan/)\n",
			PACKAGE_STRING, num_hosts);



	/*
	 *      Main loop: send packets to all hosts in order until a response
	 *      has been received or the host has exhausted its retry limit.
	 *
	 *      The loop exits when all hosts have either responded or timed out.
	 */
	reset_cum_err = 1;
	req_interval = interval;
	while (live_count) {
		if (debug) {print_times(); printf("main: Top of loop.\n");}
		/*
		 *      Obtain current time and calculate deltas since last packet and
		 *      last packet to this host.
		 */
		Gettimeofday(&now);
		/*
		 *      If the last packet was sent more than interval us ago, then we can
		 *      potentially send a packet to the current host.
		 */
		timeval_diff(&now, &last_packet_time, &diff);
		loop_timediff = (ARP_UINT64)1000000*diff.tv_sec + diff.tv_usec;
		if (loop_timediff >= (unsigned)req_interval) {
			if (debug) {print_times(); printf("main: Can send packet now. loop_timediff=" ARP_UINT64_FORMAT ", req_interval=%d, cum_err=%d\n", loop_timediff, req_interval, cum_err);}
			/*
			 *      If the last packet to this host was sent more than the current
			 *      timeout for this host us ago, then we can potentially send a packet
			 *      to it.
			 */
			timeval_diff(&now, &((*cursor)->last_send_time), &diff);
			host_timediff = (ARP_UINT64)1000000*diff.tv_sec + diff.tv_usec;
			if (host_timediff >= (*cursor)->timeout) {
				if (reset_cum_err) {
					if (debug) {print_times(); printf("main: Reset cum_err\n");}
					cum_err = 0;
					req_interval = interval;
					reset_cum_err = 0;
				} else {
					cum_err += loop_timediff - interval;
					if (req_interval >= cum_err) {
						req_interval = req_interval - cum_err;
					} else {
						req_interval = 0;
					}
				}
				if (debug) {print_times(); printf("main: Can send packet to host %s now. host_timediff=" ARP_UINT64_FORMAT ", timeout=%u, req_interval=%d, cum_err=%d\n", my_ntoa((*cursor)->addr), host_timediff, (*cursor)->timeout, req_interval, cum_err);}
				select_timeout = req_interval;
				/*
				 *      If we've exceeded our retry limit, then this host has timed out so
				 *      remove it from the list. Otherwise, increase the timeout by the
				 *      backoff factor if this is not the first packet sent to this host
				 *      and send a packet.
				 */
				if (verbose && (*cursor)->num_sent > pass_no) {
					warn_msg("---\tPass %d complete", pass_no+1);
					pass_no = (*cursor)->num_sent;
				}
				if ((*cursor)->num_sent >= retry) {
					if (verbose > 1)
						warn_msg("---\tRemoving host %s - Timeout",
								my_ntoa((*cursor)->addr));
					if (debug) {print_times(); printf("main: Timing out host %s.\n", my_ntoa((*cursor)->addr));}
					remove_host(cursor);     /* Automatically calls advance_cursor() */
					if (first_timeout) {
						timeval_diff(&now, &((*cursor)->last_send_time), &diff);
						host_timediff = (ARP_UINT64)1000000*diff.tv_sec +
							diff.tv_usec;
						while (host_timediff >= (*cursor)->timeout && live_count) {
							if ((*cursor)->live) {
								if (verbose > 1)
									warn_msg("---\tRemoving host %s - Catch-Up Timeout",
											my_ntoa((*cursor)->addr));
								remove_host(cursor);
							} else {
								advance_cursor();
							}
							timeval_diff(&now, &((*cursor)->last_send_time), &diff);
							host_timediff = (ARP_UINT64)1000000*diff.tv_sec +
								diff.tv_usec;
						}
						first_timeout=0;
					}
					Gettimeofday(&last_packet_time);
				} else {    /* Retry limit not reached for this host */
					if ((*cursor)->num_sent)
						(*cursor)->timeout *= backoff_factor;
					send_packet(pcap_handle, *cursor, &last_packet_time);
					advance_cursor();
				}
			} else {       /* We can't send a packet to this host yet */
				/*
				 *      Note that there is no point calling advance_cursor() here because if
				 *      host n is not ready to send, then host n+1 will not be ready either.
				 */
				select_timeout = (*cursor)->timeout - host_timediff;
				reset_cum_err = 1;  /* Zero cumulative error */
				if (debug) {print_times(); printf("main: Can't send packet to host %s yet. host_timediff=" ARP_UINT64_FORMAT "\n", my_ntoa((*cursor)->addr), host_timediff);}
			} /* End If */
		} else {          /* We can't send a packet yet */
			select_timeout = req_interval - loop_timediff;
			if (debug) {print_times(); printf("main: Can't send packet yet. loop_timediff=" ARP_UINT64_FORMAT "\n", loop_timediff);}
		} /* End If */
		recvfrom_wto(pcap_fd, select_timeout, pcap_handle);
	} /* End While */

	printf("\n");        /* Ensure we have a blank line */

	clean_up(pcap_handle);


	Gettimeofday(&end_time);
	timeval_diff(&end_time, &start_time, &elapsed_time);
	elapsed_seconds = (elapsed_time.tv_sec*1000 +
			elapsed_time.tv_usec/1000) / 1000.0;

	printf("Ending %s: %u hosts scanned in %.3f seconds (%.2f hosts/sec). %u responded\n",
			PACKAGE_STRING, num_hosts, elapsed_seconds,
			num_hosts/elapsed_seconds, responders);

	printf("hello world!\n");
	return 0;
}