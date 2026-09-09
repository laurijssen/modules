#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/sched/signal.h> 
#include <linux/sched/task.h>

#define NAME "ghost_task"

static int task_fn(void *data)
{
    int i;
    for (i = 0; i < 100 && !kthread_should_stop(); i++) {
        pr_info("task running: iteration %d\n", i);
        ssleep(1);
    }
    return 0;
}

static struct task_struct *find_task_by_name(const char *name)
{
	struct task_struct *task;
	for_each_process(task) {
		char task_name[TASK_COMM_LEN];
		get_task_comm(task_name, task);
		if (strcmp(task_name, name) == 0) {
			return task;
		}
	}
	return NULL;
}

static struct task_struct *new_task;

static int __init tasker_init(void)
{
	struct task_struct *task;

	new_task = kthread_create(task_fn, NULL, NAME);

	if (new_task) {
		wake_up_process(new_task);
	}

	task = find_task_by_name(NAME);

	if (task != NULL)
		pr_info("ghost task found: %s (pid: %d)\n", task->comm, task->pid);

	return 0;
}

static void __exit tasker_exit(void)
{
	struct task_struct *task = find_task_by_name(NAME);

	if (task != NULL) {
		char name[TASK_COMM_LEN];
		get_task_comm(name, task);
		pr_info("stopping ghost task: %s (pid: %d)\n", name, task->pid);
		kthread_stop(task);
	}
}

module_init(tasker_init);
module_exit(tasker_exit);

MODULE_AUTHOR("serve laurijssen");
MODULE_DESCRIPTION("experimental " NAME " injector");
MODULE_LICENSE("GPLv2");
MODULE_VERSION("0.1");
