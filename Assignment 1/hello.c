#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/syscalls.h>

SYSCALL_DEFINE0(hello)
{
	printk(KERN_INFO "Hello, world!\n");
	printk(KERN_INFO "311551137\n");
	return 0;
}

