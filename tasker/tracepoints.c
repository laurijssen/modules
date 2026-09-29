#include <linux/tracepoint.h>

#include "tracepoints.h"

static const char *exception_list[] = {
	"docker", 
	"cpuUsage.sh", 
	NULL
};

static struct tracepoint *my_tracepoint;

static void fork_tracepoint(void *data, struct task_struct *parent, struct task_struct *child);
static void find_tracepoint(struct tracepoint *tp, void *priv);

int init_sched_process_fork_tracepoint(void)
{
    for_each_kernel_tracepoint(find_tracepoint, &my_tracepoint);

    return tracepoint_probe_register(my_tracepoint, fork_tracepoint, NULL);
}

void exit_sched_process_fork_tracepoint(void)
{
    tracepoint_probe_unregister(my_tracepoint, fork_tracepoint, NULL);
}

static void fork_tracepoint(void *data, struct task_struct *parent, struct task_struct *child)
{
	char childname[TASK_COMM_LEN], parentname[TASK_COMM_LEN];

	get_task_comm(childname, child);
	get_task_comm(parentname, parent);

	for (int i = 0; exception_list[i] != NULL; i++) {
		if (strcmp(childname, exception_list[i]) == 0 || strcmp(parentname, exception_list[i]) == 0) {
			return; // Skip logging for this exception
		}
	}

	struct mm_struct *mm = child->mm;

	if (mm)
	{
		char *pathname, *p;

		down_read(&mm->mmap_lock);

   		if (mm->exe_file) 
		{
            pathname = kmalloc(PATH_MAX, GFP_ATOMIC);
            if (pathname) 
			{
                p = d_path(&mm->exe_file->f_path, pathname, PATH_MAX);

				pr_info("path: %s\n", p);                

				kfree(pathname);
            }
        }
		up_read(&mm->mmap_lock);
	}

	pr_info("forked: Parent=%s (PID=%d), Child=%s (PID=%d)", parentname, parent->pid, childname, child->pid);
}

static void find_tracepoint(struct tracepoint *tp, void *priv)
{
    if (tp->name && !strcmp(tp->name, "sched_process_fork"))
        *(struct tracepoint **)priv = tp;
}
