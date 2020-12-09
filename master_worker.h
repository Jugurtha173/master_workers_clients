#ifndef MASTER_WORKER_H
#define MASTER_WORKER_H

// On peut mettre ici des éléments propres au couple master/worker :
//    - des constantes pour rendre plus lisible les comunications
//    - des fonctions communes (écriture dans un tube, ...)

#define _XOPEN_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "myassert.h"

#include "mybiblio.h"

void my_pipe(int *fd);

int my_fork();

void my_execv(const char *path, char *const *argv);

void next_worker(int post, int fdIn, int fdToMaster);

#endif
