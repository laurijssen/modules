#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define NETLINK_MY_UNIT_PROTO  31
#define NLSPACE              1024

static const char *thedata = "driving on a saterday night";

int main(int argc, char **argv)
{
        (void)argc;
	int sd;
	struct sockaddr_nl src_nl, dest_nl;
	struct nlmsghdr *nlhdr = {0};
	struct iovec iov = {0};
	struct msghdr msg = {0};
	ssize_t nsent, nrecv;

	/* 1. Get ourselves an endpoint - a netlink socket! */
	sd = socket(PF_NETLINK, SOCK_RAW, NETLINK_MY_UNIT_PROTO);
	if (sd < 0) {
		perror("netlink_u: netlink socket creation failed");
		exit(EXIT_FAILURE);
	}
	printf("%s:PID %d: netlink socket created\n", argv[0], getpid());

	/* 2. Setup the netlink source addr structure and bind it */
	memset(&src_nl, 0, sizeof(src_nl));
	src_nl.nl_family = AF_NETLINK;
	src_nl.nl_pid = getpid();
	src_nl.nl_groups = 0x0;   // no multicast
	if (bind(sd, (struct sockaddr *)&src_nl, sizeof(src_nl)) < 0) {
		perror("netlink_u: bind failed");
		exit(EXIT_FAILURE);
	}
	printf("%s: bind done\n", argv[0]);
		
	/* 3. Setup the netlink destination addr structure */
	memset(&dest_nl, 0, sizeof(dest_nl));
	dest_nl.nl_family = AF_NETLINK;
	dest_nl.nl_groups = 0x0; // no multicast
	dest_nl.nl_pid = 0;      // destined for the kernel

	/* 4. Allocate and setup the netlink header (including the payload) */
	nlhdr = (struct nlmsghdr *)malloc(NLMSG_SPACE(NLSPACE));
	if (!nlhdr) {
		fprintf(stderr, "netlink_u: malloc nlhdr failed");
		exit(EXIT_FAILURE);
	}
	nlhdr->nlmsg_len = NLMSG_SPACE(NLSPACE);
	nlhdr->nlmsg_pid = getpid();

	strncpy(NLMSG_DATA(nlhdr), thedata, strnlen(thedata, NLSPACE)+1);
	printf("%s: destination struct, netlink hdr, payload setup\n", argv[0]);

	iov.iov_base = (void *)nlhdr;
	iov.iov_len = nlhdr->nlmsg_len;
	printf("%s: initialized iov structure (nl header folded in)\n", argv[0]);

	msg.msg_name = (void *)&dest_nl;   // dest addr
	msg.msg_namelen = sizeof(dest_nl); // size of dest addr
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1; // # elements in msg_iov
	printf("%s: initialized msghdr structure (iov folded in)\n", argv[0]);
	
	nsent = sendmsg(sd, &msg, 0);
	if (nsent < 0) {
		perror("netlink_u: sendmsg(2) failed");
		free(nlhdr);
		exit(EXIT_FAILURE);
	} else if (nsent == 0) {
		printf(" 0 bytes sent\n");
		free(nlhdr);
		exit(EXIT_FAILURE);
	}
	printf("%s:sendmsg(): *** success, sent %ld bytes all-inclusive\n (see kernel log for dtl)\n", argv[0], nsent);
	fflush(stdout);

	printf("%s: now blocking on kernel netlink msg via recvmsg() ...\n", argv[0]);
	nrecv = recvmsg(sd, &msg, 0);
	if (nrecv < 0) {
		perror("netlink_u: recvmsg(2) failed");
		free(nlhdr);
		exit(EXIT_FAILURE);
	}
	printf("%s:recvmsg(): *** success, received %ld bytes:"
		"\nmsg from kernel netlink: \"%s\"\n",
		argv[0], nrecv, (char *)NLMSG_DATA(nlhdr));

	/* Shut shop */
	free(nlhdr);
	close(sd);
	exit(EXIT_SUCCESS);
}

