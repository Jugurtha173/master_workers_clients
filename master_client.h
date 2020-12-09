#ifndef CLIENT_CRIBLE
#define CLIENT_CRIBLE


// On peut mettre ici des éléments propres au couple master/client :
//    - des constantes pour rendre plus lisible les comunications
//    - des fonctions communes (création tubes, écriture dans un tube,
//      manipulation de sémaphores, ...)

// ordres possibles pour le master
#define ORDER_NONE                0
#define ORDER_STOP               -1
#define ORDER_COMPUTE_PRIME       1
#define ORDER_HOW_MANY_PRIME      2
#define ORDER_HIGHEST_PRIME       3
#define ORDER_COMPUTE_PRIME_LOCAL 4   // ne concerne pas le master

#define _XOPEN_SOURCE
#define _POSIX_C_SOURCE 200809L
// includes
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <math.h>
#include "myassert.h"

// fichier (qui doit être accessible) choisi pour l'identification du sémaphore
#define MON_FICHIER "master_client.h"

// identifiant pour le deuxième paramètre de ftok
#define PROJ_ID_1 5
#define PROJ_ID_2 25

// les tubes
#define TCM "TUBE_CLIENT_MASTER"
#define TMC "TUBE_MASTER_CLIENT"

// bref n'hésitez à mettre nombre de fonctions avec des noms explicites
// pour masquer l'implémentation
#include "mybiblio.h"

void is_prime_local(int number);

void my_mkfifo(const char *path);

void my_unlink(const char *path);

int my_semget_key(const char *pathname, int proj_id, int nsems, int semflg);

void my_semctl(int fd, int value);

void my_semdelete(int fd);

void prendre(int semId);

void vendre(int semId);

#endif
