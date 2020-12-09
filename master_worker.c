#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>

#include "mybiblio.h"
#include "master_worker.h"

// fonctions éventuelles proposées dans le .h


void my_pipe(int *fd){
    int p = pipe(fd);
    myassert( p == 0, "ERREUR my_pipe\n");
}

int my_fork(){
    int ret = fork();
    myassert(ret != (-1), "ERREUR my_fork\n");
    return ret;
}

void my_execv(const char *path, char *const *argv){
    int e = execv(path, argv);
    myassert(e != (-1), "ERREUR my_execv\n");
}

void next_worker(int post, int fdIn, int fdToMaster){

    char* argv[5];
    argv[0] = "worker";

    char cmd_post[10];
    sprintf(cmd_post, "%d", post);
    argv[1] = cmd_post;

    char cmd_fdIn[10];
    sprintf(cmd_fdIn, "%d", fdIn);
    argv[2] = cmd_fdIn;

    char cmd_fdToMaster[10];
    sprintf(cmd_fdToMaster, "%d", fdToMaster);
    argv[3] = cmd_fdToMaster;

    argv[4] = NULL;

    my_execv(argv[0], argv);
    perror("ERREUR my_execv\n");
}


