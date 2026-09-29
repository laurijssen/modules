#include <linux/sched/task.h>
#include <trace/events/sched.h>
#include <linux/kthread.h>
#include <linux/delay.h>


#include "ghosttask.h"

static int task_fn(void *data);
static struct task_struct *find_task_by_name(const char *name);

static struct task_struct *new_task;

void start_ghost_task(const char *name)
{
	struct task_struct *task;

	new_task = kthread_create(task_fn, NULL, name);

	if (new_task) {
		wake_up_process(new_task);
	}

	task = find_task_by_name(name);

	if (task != NULL)
		pr_info("ghost task found: %s (pid: %d)\n", task->comm, task->pid);
}

void stop_ghost_task(const char *name)
{
	struct task_struct *task = find_task_by_name(name);

	if (task != NULL) {
		char name_from_task[TASK_COMM_LEN];
		get_task_comm(name_from_task, task);
		pr_info("stopping ghost task: %s (pid: %d)\n", name_from_task, task->pid);
		kthread_stop(task);
	}
	else
	{
		pr_info("task already stopped: %s\n", name);
	}

}

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