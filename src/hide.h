#ifndef HIDE_H
#define HIDE_H

void hide_self(void);
void unhide_self(void);
void hide_add_pid(pid_t pid);
int hide_should_filter(const char *name);

#endif
