/*  
 *  yubo@yubo.org
 *  2016-03-05
 */
#include <linux/module.h>   /* Needed by all modules */
#include <linux/kernel.h>   /* Needed for KERN_ALERT */
#include <linux/tcp.h>		/* for tcphdr */
#include <net/ip.h>
#include <net/tcp.h>		/* for csum_tcpudp_magic */
#include <net/udp.h>
#include <net/icmp.h>		/* for icmp_send */
#include <net/route.h>		/* for ip_route_output */
#include <net/ipv6.h>
#include <net/ip6_route.h>
#include <linux/icmpv6.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>

#include <net/ip_vs.h>
#include <linux/if_arp.h>

#define TIMER_INTERVAL	msecs_to_jiffies(1000)	/* 1s */

int timer_hash_index = 0;
static struct timer_list timer;
static __u16 seq = 0;

static struct rtable * get_rt(__be32 ip, u32 rtos){
	struct flowi fl = {
		.oif = 0,
		.nl_u = {
			.ip4_u = {
				.daddr = ip,
				.saddr = 0,
				.tos = rtos,}},
	};
	struct rtable *rt;	/* Route to the other host */

	if (ip_route_output_key(&init_net, &rt, &fl)) {
		printk("ip_route_output error, dest: %pI4\n", &ip);
		return NULL;
	}

	return rt;
}

struct stoa_addr {
	__u8 opcode;
	__u8 opsize;
	__u16 port;
	__u32 addr;
};

static void send_icmp_cb(unsigned long data)
{
	struct sk_buff *skb;
	struct iphdr *iph;
	struct ethhdr *eth;
	struct icmphdr *icmph;
	struct stoa_addr *stoa;
	unsigned int icmp_offset;
	struct rtable *rt;	/* Route to the other host */
	int ret;


	skb = alloc_skb(sizeof(*iph) + sizeof(*eth)+
			sizeof(*icmph) + sizeof(*stoa), GFP_ATOMIC);
	if (skb == NULL)
		return ;
	skb_reserve(skb, NET_IP_ALIGN+sizeof(*eth));

	// l2
	//eth = (struct ethhdr *)skb_put(skb, sizeof(*eth));

	// l3
	//skb_set_network_header(skb, sizeof(*eth));
	skb_set_network_header(skb, 0);
	iph = (struct iphdr *)skb_put(skb, sizeof(*iph));
	iph->version    = 4;
	iph->ihl        = sizeof(*iph) / 4;
	iph->tos        = 0;
	iph->id         = 0;
	iph->frag_off   = htons(IP_DF);
	iph->ttl        = 60;
	iph->protocol   = IPPROTO_ICMP;
	iph->check      = 0;
	iph->saddr = 0x02010101;
	iph->daddr = 0x01010101;
	iph->tot_len = htons(sizeof(struct iphdr) + sizeof(struct icmphdr) +
			sizeof(struct stoa_addr));
	iph->check = 0;
	ip_send_check(iph);

	// icmp
	icmph = (struct icmphdr *)skb_put(skb, sizeof(*icmph));
	icmph->type = ICMP_ECHO;
	icmph->code = 0;
	icmph->un.echo.id = htons(123);
	icmph->un.echo.sequence = htons(seq++);
	icmp_offset = (unsigned char *)icmph - skb->data;


	// stoa
	stoa = (struct stoa_addr *)skb_put(skb, sizeof(*stoa));
	stoa->opcode=254;
	stoa->opsize=8;
	stoa->addr=0x06070809;
	stoa->port=ntohs(80);



	icmph->checksum = 0;
	icmph->checksum = csum_fold(skb_checksum(skb, icmp_offset,
				skb->len - icmp_offset, 0));


	rt = get_rt(iph->daddr, RT_TOS(ip_hdr(skb)->tos));
	if(!rt){
		printk("get_rt error.\n");
		goto error_out;
	}

	skb->local_df = 1;
	skb->ipvs_property = 1;
	skb_forward_csum(skb);

	//ignore mtu checking
	//skb_dst_drop(skb);
	skb_dst_set(skb, &rt->u.dst);
	//skb->rtable = rt;
	//ret = ip_local_out(skb);
	printk("rt->u.dst.dev:%s ret:%d\n", netdev_name(rt->u.dst.dev), ret);

	// xmit

	NF_HOOK(PF_INET, NF_INET_LOCAL_OUT, skb, NULL,
			rt->u.dst.dev, dst_output);

	//ip_rt_put(rt);

error_out:
	mod_timer(&timer, jiffies + TIMER_INTERVAL);
	return;
}

int init_module(void)
{
	printk("<1>Hello world 1.\n");

	setup_timer(&timer, send_icmp_cb, 0);
	mod_timer(&timer, jiffies + TIMER_INTERVAL);

	/* 
	 * A non 0 return means init_module failed; module can't be loaded. 
	 */
	return 0;
}

void cleanup_module(void)
{
	int ret;

	ret = del_timer_sync(&timer);
	if (ret)
		printk("Timer is still in use...\n");

	printk(KERN_ALERT "Goodbye world 1.\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yubo <yubo@yubo.org>");
MODULE_DESCRIPTION("send sample icmp");
