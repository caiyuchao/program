/*
 * sample netlink module
 */
#define KMSG_COMPONENT "sampleNetlink"
#define pr_fmt(fmt) KMSG_COMPONENT ": " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <net/genetlink.h>


#include <net/ip.h>
#include <net/tcp.h>
#include <net/netfilter/nf_conntrack.h>
#include "sample_nl.h"

#define MODULE_NAME "SampleNetlink"

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
#define NLA_PUT_U32 nla_put_u32
#define NLA_PUT_STRING nla_put_string
#endif

static struct timer_list sample_timer;
struct sample_nla _sn = {
	.info = {.x = 1, .y = 2},
	.data = {'h','e','l','l','o','\0'},
	};


struct genl_family family = {
	.id	= GENL_ID_GENERATE,
	.name	 = SAMPLE_NL_FAMILY_NAME,
	.version = 1,
	.maxattr = SAMPLE_NL_CMD_MAX,
};

struct genl_multicast_group grp[] = {
	{.name	= SAMPLE_NL_GRP_NAME},
};

/* Policy used for attributes in nested attribute SAMPLE_ECHO_ATTR */
static const struct nla_policy sample_echo_policy[SAMPLE_ECHO_ATTR_MAX + 1] = {
	[SAMPLE_ECHO_ATTR_INFO] = {.type = NLA_NESTED},
	[SAMPLE_ECHO_ATTR_DATA] = {.type = NLA_NUL_STRING, .len = 2048},
};

static const struct nla_policy sample_info_policy[SAMPLE_INFO_ATTR_MAX + 1] = {
	[SAMPLE_INFO_ATTR_X] = {.type = NLA_U32},
	[SAMPLE_INFO_ATTR_Y] = {.type = NLA_U32},
};


static int sample_parse_info(struct sample_info *si, struct nlattr *nla){
	struct nlattr *attrs[SAMPLE_INFO_ATTR_MAX + 1];

	if(nla == NULL || 
			nla_parse_nested(attrs, SAMPLE_INFO_ATTR_MAX, nla, sample_info_policy))
		return -EINVAL;

	if(!attrs[SAMPLE_INFO_ATTR_X] || !attrs[SAMPLE_INFO_ATTR_Y])
		return -EINVAL;

	memset(si, 0, sizeof(*si));
	si->x = nla_get_u32(attrs[SAMPLE_INFO_ATTR_X]);
	si->y = nla_get_u32(attrs[SAMPLE_INFO_ATTR_Y]);
	return 0;
}

static int sample_parse_echo(struct sample_nla *sn, struct nlattr *nla){
	struct nlattr *attrs[SAMPLE_ECHO_ATTR_MAX + 1];

	if(nla == NULL || 
			nla_parse_nested(attrs, SAMPLE_ECHO_ATTR_MAX, nla, sample_echo_policy))
		return -EINVAL;

	if(!attrs[SAMPLE_ECHO_ATTR_DATA] || !attrs[SAMPLE_ECHO_ATTR_INFO])
		return -EINVAL;

	memset(sn, 0, sizeof(*sn));

	strcpy(sn->data, nla_data(attrs[SAMPLE_ECHO_ATTR_DATA]));
	return sample_parse_info(&sn->info, attrs[SAMPLE_ECHO_ATTR_INFO]);
}


static int sample_nl_fill_echo(struct sk_buff *skb, struct sample_nla *sn){
	struct nlattr *nl_info;

	nl_info = nla_nest_start(skb, SAMPLE_ECHO_ATTR_INFO);
	if(!nl_info)
		return -EMSGSIZE;
	NLA_PUT_U32(skb, SAMPLE_INFO_ATTR_X, sn->info.x);
	NLA_PUT_U32(skb, SAMPLE_INFO_ATTR_Y, sn->info.y);
	nla_nest_end(skb, nl_info);
	NLA_PUT_STRING(skb, SAMPLE_ECHO_ATTR_DATA, sn->data);
	return 0;

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36)
nla_put_failure:
	nla_nest_cancel(skb, nl_info);
	return -EMSGSIZE;
#endif
}

int sample_echo(struct sk_buff *skb, struct genl_info *info)
{
	int ret = 0, cmd, reply_cmd;
	struct sk_buff *msg;
	struct sample_nla sn;
	void *reply;

	cmd = info->genlhdr->cmd;
	if( cmd == SAMPLE_NL_CMD_ECHO){
		reply_cmd = SAMPLE_NL_CMD_ECHO;
	}else{
		return -EINVAL;
	}

	ret = sample_parse_echo(&sn, info->attrs[SAMPLE_CMD_ATTR_ECHO]);

	if (ret)
		goto out;

	printk("%s(%d) kernel recv echo: x[%u], y[%u], data[%s]\n", __func__, __LINE__, sn.info.x, sn.info.y, sn.data);

	//reply
	msg = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if(!msg){
		ret = -ENOMEM;
		goto out;
	}

	reply = genlmsg_put_reply(msg, info, &family, 0, reply_cmd);
	if (reply == NULL){
		ret = -EMSGSIZE;
		goto out;
	}

	if(sample_nl_fill_echo(msg, &sn)){
		ret = -EMSGSIZE;
		goto out;
	}

	genlmsg_end(msg, reply);
	ret = genlmsg_reply(msg, info);

out:
	return ret;
}

int sample_echo_dump(struct sk_buff *skb, struct netlink_callback *cb)
{	
	void *hdr;


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
	hdr = genlmsg_put(skb, NETLINK_CB(cb->skb).portid, cb->nlh->nlmsg_seq,
			&family, 0, SAMPLE_NL_CMD_ECHO);
#else
	hdr = genlmsg_put(skb, NETLINK_CB(cb->skb).pid, cb->nlh->nlmsg_seq,
			&family, 0, SAMPLE_NL_CMD_ECHO);
#endif
			//&family, NLM_F_MULTI, SAMPLE_NL_CMD_ECHO);
	if(!hdr)
		return -EMSGSIZE;

	printk("echo: x[%u], y[%u], data[%s]\n", _sn.info.x, _sn.info.y, _sn.data);
	if(sample_nl_fill_echo(skb, &_sn)){
		genlmsg_cancel(skb, hdr);
		goto nla_put_failure;
	}
	return genlmsg_end(skb, hdr);

nla_put_failure:
	genlmsg_cancel(skb, hdr);
	return -EMSGSIZE;
}

struct genl_ops ops[] = {
	{
	.cmd	= SAMPLE_NL_CMD_ECHO,
	.flags	= GENL_ADMIN_PERM, // just root
	.policy	= sample_echo_policy,
	.doit	= sample_echo,
	.dumpit	= sample_echo_dump,
	},
};

static void sample_nl_notify(struct sample_nla *sn)
{
	int ret;
	void	*hdr;
	struct sk_buff	*skb;

	skb = nlmsg_new(NLMSG_DEFAULT_SIZE, GFP_KERNEL);
	if(!skb){
		return;
	}

	hdr = genlmsg_put(skb, 0, 0, &family, 0, SAMPLE_NL_CMD_NOTIFY);
	if (!hdr)
		goto fail_fill;

	if(sample_nl_fill_echo(skb, sn)){
		ret = -EMSGSIZE;
		goto fail_fill;
	}

	if (genlmsg_end(skb, hdr) < 0)
		goto fail_fill;

	rcu_read_lock();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,36)
	ret = genlmsg_multicast_allns(&family, skb, 0, 0, GFP_ATOMIC);
#else
	ret = genlmsg_multicast_allns(skb, 0, grp[0].id, GFP_KERNEL);
#endif
	rcu_read_unlock();

	if (ret && ret != -ESRCH) {
		printk("%s: Error notifying group (%d)\n",
			__func__, ret);
		nlmsg_free(skb);
	}
	return;

fail_fill:
	printk("%s: Failed to fill nl_attr.\n", 	__func__);
	nlmsg_free(skb);
	return;
}

static void sample_timer_callback(unsigned long data)
{
	sample_nl_notify(&_sn);
	mod_timer(&sample_timer, jiffies + msecs_to_jiffies(2000));
	return;
}



static int __init sample_nl_init(void)
{
	int ret = 0;

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
	/* register the new family and all opertations but the first one */
	ret = genl_register_family_with_ops_groups(&family, ops, grp);
	if (ret) {
		printk(KERN_CRIT "%s: Could not register netlink family (%d)\n",
				__func__, ret);
		return ret;
	}
#else
	/* register the new family and all opertations but the first one */
	ret = genl_register_family(&family);
	if (ret) {
		printk(KERN_CRIT "%s: Could not register netlink family (%d)\n",
				__func__, ret);
		return ret;
	}
	ret = genl_register_ops(&family, &ops[0]);
	if (ret) {
		printk(KERN_CRIT "%s: Could not register netlink ops, ret:%d\n",
				__func__, ret);
		genl_unregister_family(&family);
		return ret;
	}

	ret = genl_register_mc_group(&family, &grp[0]);
	if (ret) {
		printk(KERN_CRIT "Module %s: group error (%d)\n",
				MODULE_NAME, ret);
		genl_unregister_ops(&family, &ops[0]);
		genl_unregister_family(&family);
		return ret;
	}
#endif
	setup_timer(&sample_timer, sample_timer_callback, 0);
	mod_timer(&sample_timer, jiffies + msecs_to_jiffies(2000));

	printk("Module %s initialized\n", MODULE_NAME);

	return 0;
}

static void __exit sample_nl_exit(void)
{
	int ret;
	ret = del_timer_sync(&sample_timer);
	if (ret)
		printk("Timer is still in use...\n");
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
	genl_unregister_family(&family);
#else
	genl_unregister_mc_group(&family, &grp[0]);
	genl_unregister_ops(&family, &ops[0]);
	genl_unregister_family(&family);
#endif
	printk("Module %s exit\n", MODULE_NAME);
}

module_init(sample_nl_init);
module_exit(sample_nl_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("yubo");
MODULE_DESCRIPTION("Sample Netlink kernel module for Linux");
