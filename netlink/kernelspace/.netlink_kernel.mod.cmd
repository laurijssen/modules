cmd_/home/kali/projects/modules/netlink/kernelspace/netlink_kernel.mod := printf '%s\n'   netlink_kernel.o | awk '!x[$$0]++ { print("/home/kali/projects/modules/netlink/kernelspace/"$$0) }' > /home/kali/projects/modules/netlink/kernelspace/netlink_kernel.mod