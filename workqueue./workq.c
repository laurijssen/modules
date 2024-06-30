#define pr_fmt(fmt) "%s:%s(): " fmt, KBUILD_MODNAME, __func__

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include "../convenient.h"

#define OURMODNAME   "workq"
#define INITIAL_VALUE	31

MODULE_AUTHOR("laurijssen");
MODULE_DESCRIPTION("kernel workqueue");
MODULE_LICENSE("Dual MIT/GPL");	
MODULE_VERSION("0.1");

static struct st_ctx {
	struct work_struct work;
	struct timer_list tmr;
	int data;
} ctx;
static unsigned long exp_ms = 420;
static u64 t1, t2;

static void ding(struct timer_list *timer)
{
	struct st_ctx *priv = from_timer(priv, timer, tmr);

	pr_debug("timed out... data=%d\n", priv->data);
	// do *NOT* put decrement inside pr_debug, then it only works when DEBUG is defined
	priv->data--;
	PRINT_CTX();

	/* until countdown done, fire it again! */
	if (priv->data)
		mod_timer(&priv->tmr, jiffies + msecs_to_jiffies(exp_ms));

	/* Now 'schedule' our workqueue function to run */
	if (!schedule_work(&priv->work))
		pr_notice("our work's already on the kernel-global workqueue!\n");
	t1 = ktime_get_real_ns();
}

/*
 * work_func() - our workqueue callback function!
 */
static void work_func(struct work_struct *work)
{
	struct st_ctx *priv = container_of(work, struct st_ctx, work);

	t2 = ktime_get_real_ns();
	pr_info("In our workq function: data=%d\n", priv->data);
	PRINT_CTX();
	SHOW_DELTA(t2, t1);
}

static int __init workq_simple_init(void)
{
	ctx.data = INITIAL_VALUE;

	INIT_WORK(&ctx.work, work_func);

	ctx.tmr.expires = jiffies + msecs_to_jiffies(exp_ms);
	ctx.tmr.flags = 0;
	timer_setup(&ctx.tmr, ding, 0);

	pr_info("Work queue initialized, timer set to expire in %ld ms\n", exp_ms);
	add_timer(&ctx.tmr); /* Arm it; lets get going! */

	return 0;		/* success */
}

static void __exit workq_simple_exit(void)
{
	// Wait for any pending work (queue) to finish
	if (cancel_work_sync(&ctx.work))
		pr_info("yes, there was indeed some pending work; now done...\n");
	/* flush_scheduled_work(); Alternative, but harder to use correctly, as well
	 * as overkill; cancel_work_[delayed_]sync() is simpler.. */

	// Wait for possible timeouts to complete... and then delete the timer
	del_timer_sync(&ctx.tmr);
	pr_info("removed\n");
}

module_init(workq_simple_init);
module_exit(workq_simple_exit);

