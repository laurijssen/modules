#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x92997ed8, "_printk" },
	{ 0x701ef4c6, "init_net" },
	{ 0x5e61663d, "__netlink_kernel_create" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x5624cd0c, "netlink_kernel_release" },
	{ 0x4629334c, "__preempt_count" },
	{ 0x98794516, "current_task" },
	{ 0xf8fea1c8, "pv_ops" },
	{ 0x7a2af7b4, "cpu_number" },
	{ 0x2cf56265, "__dynamic_pr_debug" },
	{ 0x88100ad7, "__alloc_skb" },
	{ 0x64fa2e03, "__nlmsg_put" },
	{ 0x7a72d95d, "netlink_unicast" },
	{ 0x8ddd8aad, "schedule_timeout" },
	{ 0xb83992f2, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "C557C4416E9EF5C633DF5C3");
