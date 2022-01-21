#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/init_task.h>
#include <linux/syscalls.h>
#include <asm/errno.h>

SYSCALL_DEFINE1(enter_queue, unsigned int*, w)
{
    unsigned int enter_queue_counts;
    enter_queue_counts = current->enter_queue_counter;
    if(copy_to_user(w, &enter_queue_counts, sizeof(unsigned int))){
        return -EFAULT;
    }

    return 0;
}
