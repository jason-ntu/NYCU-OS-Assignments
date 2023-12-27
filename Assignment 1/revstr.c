#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>

SYSCALL_DEFINE2(revstr, int, length, const char __user *, str)
{
    char *oribuf;
    char *revbuf;
    int i;

    // Allocate a buffer in kernel space to store the original string
    oribuf = kmalloc(length, GFP_KERNEL);
    if (!oribuf)
    {
        return -ENOMEM; // Memory allocation failed
    }

    // Copy the user-provided string to the kernel buffer
    if (copy_from_user(oribuf, str, length))
    {
        kfree(oribuf);
        return -EFAULT; // Error copying data from user space
    }

    oribuf[length] = '\0';

    // Allocate another buffer in kernel space to store the reversed string
    revbuf = kmalloc(length, GFP_KERNEL);
    if (!revbuf)
    {
        return -ENOMEM; // Memory allocation failed
    }

    // Reverse the string into the buffer
    for (i = 0; i < length; i++)
    {
        revbuf[length - 1 - i] = oribuf[i];
    }
    revbuf[length] = '\0';

    // Print the original and reversed strings
    printk(KERN_INFO "The origin string: %s\n", oribuf);
    printk(KERN_INFO "The reversed string: %s\n", revbuf);

    kfree(oribuf); // Free the kernel buffer
    kfree(revbuf);
    return 0; // Return success
}
