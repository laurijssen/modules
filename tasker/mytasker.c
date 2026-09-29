#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/sched/signal.h> 

#include "tracepoints.h"
#include "ghosttask.h"

#define NAME "ghost_task"

static char *name;

static int __init tasker_init(void)
{
	pr_info("module started");
	start_ghost_task(NAME);

	int ret = init_sched_process_fork_tracepoint();

	if (ret < 0)
	{
		pr_info("""Failed to register tracepoint probe: %d\n", ret);
	}

	return 0;
}

static void __exit tasker_exit(void)
{
	stop_ghost_task(NAME);

	exit_sched_process_fork_tracepoint();
}

module_init(tasker_init);
module_exit(tasker_exit);

MODULE_AUTHOR("serve laurijssen");
MODULE_DESCRIPTION("experimental tasks");
MODULE_LICENSE("GPL v2");
MODULE_VERSION("0.1");
