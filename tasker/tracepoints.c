#include <linux/tracepoint.h>
#include <linux/hashtable.h>
#include <linux/debugfs.h>
#include <linux/slab.h>

#include "tracepoints.h"

static DEFINE_MUTEX(tracepoint_mutex);

static struct dentry *dir, *file;

static ssize_t my_read(struct file *f, char __user *buf,
                       size_t len, loff_t *offset);

static const struct file_operations fops = {
    .read = my_read,
};

#define HASHTABLE_SIZE 16

struct h_node {
    char *process_name;
    struct hlist_node node;
};

DECLARE_HASHTABLE(process_exceptions_hashtable, HASHTABLE_SIZE);

static struct tracepoint *my_tracepoint;

static void fork_tracepoint(void *data, struct task_struct *parent, struct task_struct *child);
static void find_tracepoint(struct tracepoint *tp, void *priv);
static uint32_t hash(const char *key);

int init_sched_process_fork_tracepoint(void)
{
	dir = debugfs_create_dir("tasker", NULL);
	file = debugfs_create_file("processes", 0444, dir, NULL, &fops);

	hash_init(process_exceptions_hashtable);

    for_each_kernel_tracepoint(find_tracepoint, &my_tracepoint);

    return tracepoint_probe_register(my_tracepoint, fork_tracepoint, NULL);
}

void exit_sched_process_fork_tracepoint(void)
{
	struct h_node *cur;
	int bkt;
	struct hlist_node *tmp;

    tracepoint_probe_unregister(my_tracepoint, fork_tracepoint, NULL);

	hash_for_each_safe(process_exceptions_hashtable, bkt, tmp, cur, node) {
		pr_info("removed process %s", cur->process_name);
		hash_del(&cur->node);
		kfree(cur->process_name);
		kfree(cur);
	}

	debugfs_remove_recursive(dir);
}

static ssize_t my_read(struct file *f, char __user *buf,
                       size_t len, loff_t *offset)
{
	struct h_node *cur;
    char *tmp;
    int total = 0, bkt;

    tmp = kmalloc(PAGE_SIZE, GFP_KERNEL);
    if (!tmp)
	{
        return -ENOMEM;
	}

	mutex_lock(&tracepoint_mutex);

	hash_for_each(process_exceptions_hashtable, bkt, cur, node) 
	{
		total += scnprintf(tmp + total, PAGE_SIZE - total, "%s\n", cur->process_name);
	}    

	mutex_unlock(&tracepoint_mutex);

    int ret = simple_read_from_buffer(buf, len, offset, tmp, total);	

	kfree(tmp);

	return ret;
}

static void fork_tracepoint(void *data, struct task_struct *parent, struct task_struct *child)
{
	struct h_node *cur;

	char childname[TASK_COMM_LEN];

	get_task_comm(childname, child);

	uint32_t keychild = hash(childname);

	hash_for_each_possible(process_exceptions_hashtable, cur, node, keychild) 
	{
        //pr_info("exceptions: match for key %u: data = %s\n", keychild, cur->process_name);

        if (strcmp(cur->process_name, childname) == 0) {
            //pr_info("myhashtable: element named \"%s\" found!", childname);
            return;
        }
    }	

	struct h_node *newnode = kmalloc(sizeof *newnode, GFP_KERNEL);

	newnode->process_name = kmalloc(strlen(childname) + 1, GFP_KERNEL);
	strcpy(newnode->process_name, childname);

	mutex_lock((&tracepoint_mutex));

	hash_add(process_exceptions_hashtable, &newnode->node, keychild);

	mutex_unlock(&tracepoint_mutex);

	struct mm_struct *mm = child->mm;

	if (mm)
	{
		char *pathname;

		mmap_read_lock(mm);

   		if (mm->exe_file) 
		{
            pathname = kmalloc(PATH_MAX, GFP_ATOMIC);
            if (pathname) 
			{
                char *p = d_path(&mm->exe_file->f_path, pathname, PATH_MAX);

				pr_info("path: %s\n", p);                

				kfree(pathname);
            }
        }
		mmap_read_unlock(mm);
	}

	pr_info("forked: child=%s (PID=%d)", childname, child->pid);
}

static void find_tracepoint(struct tracepoint *tp, void *priv)
{
    if (tp->name && !strcmp(tp->name, "sched_process_fork"))
        *(struct tracepoint **)priv = tp;
}

static uint32_t hash(const char *key) {
    unsigned long h = 0;
    while (*key)
        h = (h << 5) + h + *key++;  // djb2-like hash
    return h % HASHTABLE_SIZE;
}
