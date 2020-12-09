#define _XOPEN_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sem.h>
#include "myassert.h" 

void my_write(int fd, void *buf, size_t nbytes){
    int w = write(fd, buf, nbytes);
    myassert(w != (-1), "ERREUR my_write\n");
}

void my_read(int fd, void *buf, size_t nbytes){
    int r = read(fd, buf, nbytes);
    myassert(r != (-1), "ERREUR my_read\n");
}

int my_open(const char *file, int flag){
    int o = open(file, flag);
    myassert(o != (-1), "REEUR my_open\n");
    return o;
}

void my_close(int fd){
    int c = close(fd);
    myassert(c != (-1), "ERREUR my_close\n");
}
