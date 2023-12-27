#include <linux/atomic.h> 
#include <linux/cdev.h> 
#include <linux/delay.h> 
#include <linux/device.h> 
#include <linux/fs.h> 
#include <linux/init.h> 
#include <linux/kernel.h> /* for sprintf() */ 
#include <linux/module.h> 
#include <linux/printk.h> 
#include <linux/types.h> 
#include <linux/uaccess.h> /* for get_user and put_user */ 
#include <linux/version.h> 
#include <linux/utsname.h>
#include <linux/processor.h>
#include <linux/mm.h>
#include <linux/taskstats.h>
#include <linux/time.h>
#include <linux/time_namespace.h>

#include <asm/errno.h> 
#include "kfetch.h"

/*  Prototypes - this would normally go in a .h file */ 
static int kfetch_open(struct inode *, struct file *); 
static int kfetch_release(struct inode *, struct file *); 
static ssize_t kfetch_read(struct file *, char __user *, size_t, loff_t *); 
static ssize_t kfetch_write(struct file *, const char __user *, size_t, loff_t *); 
 
#define SUCCESS 0 
#define LINE_NUM 8
#define MAX_LINE_LEN 128
 
/* Global variables are declared as static, so are global within the file. */ 
 
static int major; /* major number assigned to our device driver */ 
static int mask;
 
enum { 
    CDEV_NOT_USED = 0, 
    CDEV_EXCLUSIVE_OPEN = 1, 
}; 
 
/* Is device open? Used to prevent multiple access to device */ 
static atomic_t already_open = ATOMIC_INIT(CDEV_NOT_USED); 
 
static struct class *cls;

const static struct file_operations kfetch_fops = {
    .owner   = THIS_MODULE,
    .read    = kfetch_read,
    .write   = kfetch_write,
    .open    = kfetch_open,
    .release = kfetch_release,
};

static char kfetch_buf[KFETCH_BUF_SIZE];
 
static int __init kfetch_init(void) 
{ 
    major = register_chrdev(0, KFETCH_DEV_NAME, &kfetch_fops); 
 
    if (major < 0) { 
        pr_alert("Registering char device failed with %d\n", major); 
        return major; 
    } 
 
    pr_info("I was assigned major number %d.\n", major); 
 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0) 
    cls = class_create(KFETCH_DEV_NAME); 
#else 
    cls = class_create(THIS_MODULE, KFETCH_DEV_NAME); 
#endif 
    device_create(cls, NULL, MKDEV(major, 0), NULL, KFETCH_DEV_NAME); 
 
    pr_info("Device created on /dev/%s\n", KFETCH_DEV_NAME);

    mask = KFETCH_FULL_INFO;
 
    return SUCCESS; 
} 
 
static void __exit kfetch_exit(void) 
{ 
    device_destroy(cls, MKDEV(major, 0)); 
    class_destroy(cls); 
 
    /* Unregister the device */ 
    unregister_chrdev(major, KFETCH_DEV_NAME); 
} 
 
/* Methods */ 
 
/* Called when a process tries to open the device file, like "sudo cat /dev/kfetch_mod"
   Should set up protections properly
*/ 
static int kfetch_open(struct inode *inode, struct file *file) 
{ 
    if (atomic_cmpxchg(&already_open, CDEV_NOT_USED, CDEV_EXCLUSIVE_OPEN)) 
        return -EBUSY; 
 
    try_module_get(THIS_MODULE); 
 
    return SUCCESS; 
} 
 
/* Called when a process closes the device file.
 * Should clean up protections properly.
 */ 
static int kfetch_release(struct inode *inode, struct file *file) 
{

    /* We're now ready for our next caller */ 
    atomic_set(&already_open, CDEV_NOT_USED); 
 
    /* Decrement the usage count, or else once you opened the file, you will 
     * never get rid of the module. 
     */ 
    module_put(THIS_MODULE); 
 
    return SUCCESS; 
} 

/* Called when a process, which already opened the dev file, attempts to read from it.
 * Should return data that includes a logo and required informations:
 *     machine hostname
 *     separator line
 *     chosen informations by the mask:
 *         Kernel: The kernel release
 *         CPU: The CPU model name
 *         CPUs: The number of CPU cores, in the format <# of online CPUs> / <# of total CPUs>
 *         Mem: The memory information, in the format<free memory> / <total memory> (in MB)
 *         Procs: The number of processes
 *         Uptime: How long the system has been running, in minutes.
 */ 
static ssize_t kfetch_read(struct file *filp, /* see include/linux/fs.h   */ 
                           char __user *buffer, /* buffer to fill with data */ 
                           size_t length, /* length of the buffer     */ 
                           loff_t *offset) 
{ 
    char outputs[LINE_NUM][MAX_LINE_LEN] = {
        "                   ",
        "        .-.        ",
        "       (.. |       ",
        "       <>  |       ",
        "      / --- \\      ",
        "     ( |   | |     ",
        "   |\\\\_)___/\\)/\\   ",
        "  <__)------(__/   "
    };

    int cur_line;
    char buf[MAX_LINE_LEN - 19];

    /* fetching the information */

    /* hostname */
    struct new_utsname *uts;    
    uts = utsname();
    if (!uts) {
        printk(KERN_ERR "Failed to get utsname\n");
    }
    strlcat(outputs[0], uts->nodename, MAX_LINE_LEN);

    /* separator line */
    for (int i = 0; i < strlen(uts->nodename); i++) {
        strlcat(outputs[1], "-", MAX_LINE_LEN);
    }

    cur_line = 1;

    /* the kernel release */
    if (mask & KFETCH_RELEASE) {
        snprintf(buf, sizeof(buf), "Kernel:\t%s", uts->release);
        strlcat(outputs[++cur_line], buf, MAX_LINE_LEN);
    }

    /* the CPU model name */
    if (mask & KFETCH_CPU_MODEL) {
        snprintf(buf, sizeof(buf), "CPU:\t\t%s", cpu_data(0).x86_model_id);
        strlcat(outputs[++cur_line], buf, MAX_LINE_LEN);
    }

    /* the number of CPU cores, in the format <# of online CPUs> / <# of total CPUs> */
    if (mask & KFETCH_NUM_CPUS) {
        snprintf(buf, sizeof(buf), "CPUs:\t%d / %d", num_online_cpus(), num_possible_cpus());
        strlcat(outputs[++cur_line], buf, MAX_LINE_LEN);
    }

    /* the memory information, in the format<free memory> / <total memory> (in MB) */
    if (mask & KFETCH_MEM) {
        struct sysinfo si;
        si_meminfo(&si);
        snprintf(buf, sizeof(buf), "Mem:\t\t%lu MB / %lu MB", si.freeram >> (20 - PAGE_SHIFT), si.totalram >> (20 - PAGE_SHIFT));
        strlcat(outputs[++cur_line], buf, MAX_LINE_LEN);
    }

    /* the number of processes */
    if (mask & KFETCH_NUM_PROCS) {
        struct task_struct *task;
        unsigned int nr_processes = 0;
        /* Traverse the task list */
        for_each_process(task) {
            nr_processes++;
        }
        snprintf(buf, sizeof(buf), "Procs:\t%u", nr_processes);
        strlcat(outputs[++cur_line], buf, MAX_LINE_LEN);
    }

    /* how long the system has been running, in minutes */
    if (mask & KFETCH_UPTIME) {
        struct timespec64 uptime;
        ktime_get_boottime_ts64(&uptime);
	    timens_add_boottime(&uptime); 
        snprintf(buf, sizeof(buf), "Uptime:\t%lu mins", (unsigned long) uptime.tv_sec / 60);
        strlcat(outputs[++cur_line], buf, MAX_LINE_LEN);
    }

    /* combine the lines  */
    for(int i = 0; i < LINE_NUM; i++) {
        strlcat(outputs[i], "\n", MAX_LINE_LEN);
        strlcat(kfetch_buf, outputs[i], KFETCH_BUF_SIZE);
    }
    
    if (copy_to_user(buffer, kfetch_buf, KFETCH_BUF_SIZE - 1)) {
        pr_alert("Failed to copy data to user");
        return -1;
    }

    /* cleaning up */
    memset(kfetch_buf, 0, KFETCH_BUF_SIZE);

    return KFETCH_BUF_SIZE;
} 
 
/* Called when a process writes to dev file: echo "hi" > /dev/hello
 * Should set the information mask in the module,
 * which determines what data is returned by the read operation.
 */ 
static ssize_t kfetch_write(struct file *filp, const char __user *buffer, 
                            size_t length, loff_t *offset) 
{ 
    int mask_info;

    if (copy_from_user(&mask_info, buffer, length)) {
        pr_alert("Failed to copy data from user");
        return 0;
    }

    /* setting the information mask */
    mask = mask_info;

    return 0; 
} 
 
module_init(kfetch_init); 
module_exit(kfetch_exit); 
 
MODULE_LICENSE("GPL");