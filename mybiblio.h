#ifndef MY_BIBLIO_H
#define MY_BIBLIO_H

#include <stdlib.h>
#include <wait.h>

void my_write(int fd, void *buf, size_t nbytes);

void my_read(int fd, void *buf, size_t nbytes);

int my_open(const char *file, int flag);

void my_close(int fd);


#endif
