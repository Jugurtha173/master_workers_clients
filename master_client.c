#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "myassert.h"
#include "mybiblio.h"
#include "master_client.h"

// fonctions éventuelles proposées dans le .h

typedef struct
{
  int number;
  int index;
  bool *tabAtIndex;
} thread_data;

void *my_function(void *arg)
{
    thread_data *td = (thread_data *)arg;
    if((td->number % td->index) == 0 && td->number != 2)
        (*td->tabAtIndex) = false;
    else
        (*td->tabAtIndex) = true;

    return NULL;
}

void is_prime_local(int number)
{

    printf("Lancement des threads\n");

    int size = (sqrt(number));
    bool tab[size];
    pthread_t tab_threads[size];
    thread_data datas[size];
    // pré-initialisation des données
    for (int i = 0; i < size; i++)
    {
        datas[i].number = number;
        datas[i].index = i + 2;
        datas[i].tabAtIndex = &tab[i];
    }
    for (int i = 0; i < size; i++)
    {
        int create = pthread_create(&tab_threads[i], NULL, my_function, &datas[i]);
        myassert(create == 0, "ERREUR pthread_create\n");        
    }
    for (int i = 0; i < size; i++)
    {
        int join = pthread_join(tab_threads[i], NULL);
        myassert(join == 0, "ERREUR jion\n");
    }

    char * reponse = "est premier\n";
    for (int i = 0; i < size; i++)
    {
        if(!(tab[i]))
        {
            reponse = "n'est pas premier\n";
            break;
        }
    }
    printf("Le numero %d %s", number, reponse);
}

void my_mkfifo(const char *path){
    int f = mkfifo(path, 0666);
    myassert(f == 0, "ERREUR my_mkfifo\n");
}

void my_unlink(const char *path){
    int u = unlink(path);
    myassert(u != (-1), "ERREUR my_unlink\n");
}

int my_semget_key(const char *pathname, int proj_id, int nsems, int semflg){
    // creation d'un nouvelle clé unique
    key_t KEY = ftok(pathname, proj_id);
    myassert(KEY >= 0, "ERREUR my_semget_key : ftok() fail\n");
    // creation du semaphore
    int sid = semget(KEY, nsems, semflg);
    myassert(sid >= 0, "ERREUR my_semget_key : semget() fail\n");

    return sid;
}

void my_semctl(int fd, int value){
    int sctl = semctl(fd, 0, SETVAL, value);
    myassert(sctl >= 0, "ERREUR my_semctl\n");
}

void my_semdelete(int fd){
    int d = semctl(fd, -1, IPC_RMID);
    myassert(d >= 0, "ERREUR my_semdelete\n");
}

void prendre(int semId){
    struct sembuf prendre = {0,-1,0};
    int p = semop(semId, &prendre,1);
    myassert(p >= 0, "ERREUR : prendre\n");
}

void vendre(int semId){
    struct sembuf vendre = {0,1,0};
    int v = semop(semId, &vendre,1);
    myassert(v >= 0, "ERREUR : vendre\n");
}
