#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include "../../convenient.h"

MODULE_AUTHOR("laurijssen");
MODULE_DESCRIPTION("netlink recv/send kernel module");
MODULE_LICENSE("Dual MIT/GPL");
MODULE_VERSION("0.1");

#define OURMODNAME   "netlink_kernel"
#define NETLINK_PROTO_NUMBER   31
#define NLSPACE              1024

static struct sock *nlsock;

static void netlink_recv_and_reply(struct sk_buff *skb)
{
	struct nlmsghdr *nlh;
	struct sk_buff *skb_tx;
	char *reply = "doing alright";
	int pid, msgsz, stat;

	PRINT_CTX();

	nlh = (struct nlmsghdr *)skb->data;
	pid = nlh->nlmsg_pid;
	pr_info("received from PID %d:\n\"%s\"\n", pid, (char *)NLMSG_DATA(nlh));

	msgsz = strnlen(reply, NLSPACE);
	skb_tx = nlmsg_new(msgsz, 0);
	if (!skb_tx) {
		pr_warn("skb alloc failed!\n");
		return;
	}

	nlh = nlmsg_put(skb_tx, 0, 0, NLMSG_DONE, msgsz, 0);
	NETLINK_CB(skb_tx).dst_group = 0;
	strncpy(nlmsg_data(nlh), reply, msgsz);

	stat = nlmsg_unicast(nlsock, skb_tx, pid);
	if (stat < 0)
		pr_warn("nlmsg_unicast() failed (err=%d)\n", stat);
	pr_info("reply sent\n");
}

static struct netlink_kernel_cfg nl_kernel_cfg = {
	.input = netlink_recv_and_reply,
};

static int __init netlink_init(void)
{
	pr_info("creating kernel netlink socket\n");

	/* struct sock *
	 * netlink_kernel_create(struct net *net, int unit,
	 *	struct netlink_kernel_cfg *cfg) */
	nlsock = netlink_kernel_create(&init_net, NETLINK_PROTO_NUMBER, &nl_kernel_cfg);
	if (!nlsock) {
		pr_warn("netlink_kernel_create failed\n");
		return PTR_ERR(nlsock);
	}

	pr_info("inserted netlink kernel module\n");
	return 0;
}

static void __exit netlink_exit(void)
{
	netlink_kernel_release(nlsock);
	pr_info("removed netlink comm module\n");
}

module_init(netlink_init);
module_exit(netlink_exit);

