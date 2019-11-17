#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/crypto.h>
#include <linux/random.h>

#include <crypto/rng.h>
#define DEVICE_NAME "randomDevice"
#define BUFFER_SIZE 131072 // Buffer size >= 2 ^ 17 is required


static dev_t first;

static struct cdev c_dev;
static struct class *c1;

static char result[BUFFER_SIZE];

// dummy open (no use)
static int my_open(struct inode *i, struct file *f) {
    printk(KERN_INFO "Driver: open\n");
    return 0;
}

// dummy close (no use)
static int my_close(struct inode *i, struct file *f) {
    printk(KERN_INFO "Driver: close\n");
    return 0;
}

// Reverse a string
static void reverse(char *str, long len) {
    long i;
    for (i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

// Convert `value` from uint to string `buffer` with size `len`
static void _itoa(uint value, char *buffer, long len) {
    long i = 0;

    while (i < len && value > 0) {
        int c = value % 10;
        value /= 10;
        buffer[i] = (char)(c + 48);
        ++i;
    }

    if (i < len) {
        buffer[i] = '\0';
    }

    reverse(buffer, i);
}

// Handle read from an application from userspace
static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off) {
    printk(KERN_INFO "Driver: read (%lld)\n", *off);

    // Generate a random value for unsigned int `i`
    uint i;
    
    // Fill `sizeof(uint)` bytes of random values, start at &i
    get_random_bytes(&i, sizeof(uint));

    // Convert to string
    _itoa(i, result, sizeof(result));

    // Debugging
    printk(KERN_INFO "Driver: Generated number: %u - %s\n", i, result);

    // Copy from result to user buffer
    if (copy_to_user(buf, result, len) != 0) {
        // Return exit code -14
        return -EFAULT;
    }

    // Success
    return 0;
}

// dummy write (no use)
static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off) {
    printk(KERN_INFO "Driver: write\n");
    return len;
};


static struct file_operations pugs_fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_close,
    .read = my_read,
    .write = my_write
};

// Module init
static int __init random_init(void) {
    if (alloc_chrdev_region(&first, 0, 3, DEVICE_NAME) < 0) {
        return -1;
    }

    if ((c1 = class_create(THIS_MODULE, "chardrv")) == NULL) {
        unregister_chrdev_region(first, 1);
        return -1;
    }

    if (device_create(c1, NULL, first, NULL, DEVICE_NAME) == NULL) {
        class_destroy(c1);
        unregister_chrdev_region(first, 1);
        return -1;
    }

    cdev_init(&c_dev, &pugs_fops);
    if (cdev_add(&c_dev, first, 1) == -1) {
        device_destroy(c1, first);
        class_destroy(c1);
        unregister_chrdev_region(first, 1);
        return -1;
    }

    printk(KERN_INFO "Driver: Random Number Generator registered");
    return 0;
}

// Module exit 
static void __exit random_exit(void) {
    cdev_del(&c_dev);
    device_destroy(c1, first);
    class_destroy(c1);
    unregister_chrdev_region(first, 1);
    printk(KERN_INFO "Driver: Random Number Generator unregistered");
}

module_init(random_init);
module_exit(random_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Random Number Generator Character Driver");
