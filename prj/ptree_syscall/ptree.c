#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/unistd.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/pid.h>
#include <linux/init_task.h>
#include <linux/spinlock.h>
#include <linux/rwlock_types.h>
#include <linux/rwlock.h>
#include "prinfo.h"

MODULE_LICENSE("Dual BSD/GPL");

#define __NR_ptree 356

/*
 *      CS356 Operating System Project 1: Problem 1
 * This is a Linux kernel module of ptree, which takes two
 * arguments buf and nr, and stores related task_struct information
 * in buf in dfs order
 *
 *        author: Ma Yesheng 5140209064
 */



static int (*oldcall)(void);

DEFINE_RWLOCK(buf_lock);

// copy required fields from task_struct to prinfo struct
static int copy_prinfo_from_task(struct prinfo *dst, struct task_struct *src) 
{
    struct task_struct *first_child;
    struct task_struct *next_sibling;
    
    if (dst==NULL || src==NULL) {
        printk("ERROR ptree: deref NULL pointer of prinfo or task_struct\n");
        return -1;
    }
    dst->parent_pid = src->parent->pid;
    dst->pid = src->pid;
    // get first_child_pid
    if (list_empty(&src->children)) {
        dst->first_child_pid = 0;
    } else {
        first_child = list_entry((src->children).next, struct task_struct, sibling);
        dst->first_child_pid = first_child->pid;
    }
    // get next_sibling_pid
    if (list_empty(&(src->sibling))) {
        dst->next_sibling_pid = 0;
    } else {
        next_sibling = list_entry((src->sibling).next, struct task_struct, sibling);
        dst->next_sibling_pid = next_sibling->pid;
    }
    dst->state = src->state;
    dst->uid = src->cred->uid;
    get_task_comm(dst->comm, src);
    return 0;
}

// traverse through current process recursively
void dfs(struct task_struct *cur_task,
         struct prinfo *cur_buf, 
         int *cur_num, 
         const int tot) 
{
    struct task_struct *tmp; // tmp defined as an iterator
    
    if (*cur_num>=tot || cur_task == NULL)
        return;
        
    write_lock(&buf_lock);
    copy_prinfo_from_task(cur_buf+(*cur_num), cur_task);
    ++*cur_num;
    write_unlock(&buf_lock);
    
    // traverse through children
    if (!list_empty(&cur_task->children)) {
        list_for_each_entry(tmp, &cur_task->children, sibling) {
            dfs(tmp, cur_buf, cur_num, tot);
        }
    }
}

// main function of ptree syscall
static long sys_ptree(struct prinfo __user *buf, int __user *nr)
{
    struct prinfo __kernel *kern_buf;
    int __kernel kern_nr;
    int __kernel kern_tot;
    
    // check validity of arguments
    if (buf==NULL || nr==NULL) {
        printk(KERN_ERR "ERROR ptree: deref NULL pointer of buf or nr\n");
        return -1;
    }
    // copy the value of nr in user space
    if (copy_from_user(&kern_tot, nr, sizeof(int))) {
        printk(KERN_ERR "ERROR ptree: copy nr failure\n");
        return -1;
    }
    kern_nr = 0;
    
    // create kernel memory space for ptree
    kern_buf = kmalloc(kern_tot*sizeof(struct prinfo), GFP_KERNEL);
    if (kern_buf == NULL) {
        printk(KERN_ERR "ERROR ptree: kern_buf create failure\n");
        return -1;
    } else {
        printk(KERN_INFO "INFO ptree: kern_buf created sucessfully\n");
    }
    
    read_lock(&tasklist_lock);
    dfs(&init_task, kern_buf, &kern_nr, kern_tot);
    read_unlock(&tasklist_lock);
    
    // copy result to userspace
    printk(KERN_INFO "%d of prinfo memory unused", kern_tot-kern_nr);
    
    if (copy_to_user(buf, kern_buf, kern_nr*sizeof(struct prinfo))) {
        printk(KERN_ERR "ERROR ptree: copy to userspace buf failure\n");
        return -1;
    }
    if (copy_to_user(nr, &kern_nr, sizeof(int))) {
        printk(KERN_ERR "ERROR ptree: copy to userspce nr failure\n");
        return -1;
    }
    kfree(kern_buf);
    printk(KERN_INFO "INFO ptree: buf copied successfully\n");
    return 0;
}

static int addsyscall_init(void) {
    long *syscall = (long *)0xc000d8c4;
    oldcall = (int(*)(void))(syscall[__NR_ptree]);
    syscall[__NR_ptree] = (unsigned long) sys_ptree;
    printk(KERN_INFO "INFO ptree: module load\n");
    return 0;
}

static void addsyscall_exit(void) {
    long *syscall = (long *) 0xc000d8c4;
    syscall[__NR_ptree] = (unsigned long) oldcall;
    printk(KERN_INFO "INFO ptree: module exit\n");
}

module_init(addsyscall_init);
module_exit(addsyscall_exit);
