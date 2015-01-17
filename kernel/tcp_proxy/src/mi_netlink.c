/*
 * Miwifi netlink module
 */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <net/genetlink.h>


#include <net/ip.h>
#include <net/tcp.h>
#include <net/netfilter/nf_conntrack.h>
#include "mi_netlink.h"

#define MODULE_NAME "MiNetlink"

static struct sk_buff_head mi_skb_list;
static struct work_struct mi_netlink_work;

struct genl_family family = {
	.id	= GENL_ID_GENERATE,
	.name	= MI_NETLINK_NL_FAMILY_NAME,
	.version = 1,
	.maxattr = MI_NETLINK_ATTR_MAX
};

struct genl_multicast_group grp = {
	.name	= MI_NETLINK_NL_GRP_NAME,
};

int minetlink_dump (struct sk_buff *skb, struct netlink_callback *data)
{
	return 0;
}
struct genl_ops ops = {
	.cmd	= MI_NETLINK_CMD_SKB_DUMP,
	.flags	= 0,
	.policy	= NULL,
	.doit	= NULL,
	.dumpit	= minetlink_dump,
	.done	= NULL
};

static void miwifi_netlink_notify(struct genl_multicast_group *nl_grp,
	struct mi_http_info *http_info, struct sk_buff *orig_skb)
{
	int ret;
	int len;
	void	*hdr;
	struct sk_buff	*skb;

	skb = nlmsg_new(256 + orig_skb->len, GFP_KERNEL);
	if (!skb)
		return;

	hdr = genlmsg_put(skb, 0, 0, &family, 0, MI_NETLINK_CMD_SKB_DUMP);
	if (!hdr)
		goto fail_fill;

	/* todo unlinear */
	len = skb_tailroom(skb);
	if (len > orig_skb->len)
		len = orig_skb->len;
	else
		printk(KERN_DEBUG "%s: skb tailroom is no enough(%d) !!\n",
			__func__, orig_skb->len);

	if (nla_put(skb, MI_NETLINK_TYPE_HTTP_INFO,
	    sizeof(struct mi_http_info), http_info))
		goto fail_fill;

	if (nla_put(skb, MI_NETLINK_TYPE_HTTP_DATA,
	    orig_skb->len, orig_skb->data))
		goto fail_fill;

	if (genlmsg_end(skb, hdr) < 0)
		goto fail_fill;

	ret = genlmsg_multicast_allns(skb, 0, nl_grp->id, GFP_KERNEL);

	if (ret && ret != -ESRCH) {
		printk(KERN_DEBUG "%s: Error notifying group (%d)\n",
			__func__, ret);
		nlmsg_free(skb);
	}
	return;

fail_fill:
	printk(KERN_DEBUG "%s: Failed to fill nl_attr.\n", 	__func__);
	nlmsg_free(skb);
	return;
}

void mi_netlink_broadcast(struct sk_buff *orig_skb)
{
	struct sk_buff *new_skb = skb_clone(orig_skb, GFP_ATOMIC);

	if (!new_skb)
		return;

	local_bh_disable();
	skb_queue_tail(&mi_skb_list, new_skb);
	local_bh_enable();
	schedule_work(&mi_netlink_work);

	return;
}
EXPORT_SYMBOL(mi_netlink_broadcast);

static void mi_netlink_task(struct work_struct *work)
{
	struct iphdr *iph;
	struct tcphdr *th;
	struct mi_http_info http_info;
	struct sk_buff *skb;

	do {
		local_bh_disable();
		skb = skb_dequeue(&mi_skb_list);
		local_bh_enable();
		if (skb) {
			iph = ip_hdr(skb);
			if (iph->protocol != IPPROTO_TCP)
				return;
			th = (struct tcphdr *) ((char *)iph + iph->ihl*4);
			skb_pull(skb, iph->ihl*4 + th->doff*4);
			http_info.sip = iph->saddr;
			http_info.dip = iph->daddr;
			http_info.sport = th->source;
			http_info.dport = th->dest;

			miwifi_netlink_notify(&grp, &http_info, skb);
			kfree_skb(skb);
		}
	} while (skb != NULL);

	return;
}

#ifdef __MI_NETLINK_MODULE__
static int __init mi_netlink_init(void)
#else
int mi_netlink_init(void)
#endif
{
	int ret = 0;

	skb_queue_head_init(&mi_skb_list);
	/* register the new family and all opertations but the first one */
	ret = genl_register_family(&family);
	if (ret) {
		printk(KERN_CRIT "%s: Could not register netlink family (%d)\n",
				__func__, ret);
		return ret;
	}
	ret = genl_register_ops(&family, &ops);
	if (ret) {
		printk(KERN_CRIT "%s: Could not register netlink ops, ret:%d\n",
				__func__, ret);
		genl_unregister_family(&family);
		return ret;
	}

	ret = genl_register_mc_group(&family, &grp);
	if (ret) {
		printk(KERN_CRIT "Module %s: group error (%d)\n",
				MODULE_NAME, ret);
		genl_unregister_ops(&family, &ops);
		genl_unregister_family(&family);
		return ret;
	}

	INIT_WORK(&mi_netlink_work, mi_netlink_task);

	printk(KERN_DEBUG "Module %s initialized\n", MODULE_NAME);

	return 0;
}

#ifdef __MI_NETLINK_MODULE__
static void __exit mi_netlink_exit(void)
#else
void mi_netlink_exit(void)
#endif
{
	skb_queue_purge(&mi_skb_list);
	flush_work(&mi_netlink_work);
	genl_unregister_mc_group(&family, &grp);
	genl_unregister_ops(&family, &ops);
	genl_unregister_family(&family);
	printk(KERN_DEBUG "Module %s exit\n", MODULE_NAME);
}

#ifdef __MI_NETLINK_MODULE__
module_init(mi_netlink_init);
module_exit(mi_netlink_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Zhangjunwei");
MODULE_DESCRIPTION("Miwifi Netlink kernel module for Linux");
#endif
