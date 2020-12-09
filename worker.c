#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#include "myassert.h"

#include "master_worker.h"



int YES = 1;
int NO = 0;
int STOP = -1;

/************************************************************************
 * Données persistantes d'un worker
 ************************************************************************/

// on peut ici définir une structure stockant tout ce dont le worker
// a besoin : le nombre premier dont il a la charge, ...
typedef struct data{
    int p; // le nombre dont il a la charge (P)
    int fdIn; // file descriptor entrant
    int fdOut[2]; // file descriptor sortant
    int fdToMaster; // file descriptor vers le master
    bool hasNext; // pour savoir si il a un suivant
}*WORKER_DATA;

WORKER_DATA init_worker(const char *p, const char *fdIn, const char *fdToMaster){
    WORKER_DATA worker_data = malloc(sizeof(struct data));
    worker_data->p = atoi(p);
    worker_data->fdIn = atoi(fdIn);
    worker_data->fdToMaster = atoi(fdToMaster);
    worker_data->hasNext = false; // pas de suivant
/*     worker_data->fdOut = -1; // pas tube vers le suivant     */
    return worker_data;
}

/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <n> <fdIn> <fdToMaster>\n", exeName);
    fprintf(stderr, "   <n> : nombre premier géré par le worker\n");
    fprintf(stderr, "   <fdIn> : canal d'entrée pour tester un nombre\n");
    fprintf(stderr, "   <fdToMaster> : canal de sortie pour indiquer si un nombre est premier ou non\n");
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

static void parseArgs(int argc, char * argv[] , WORKER_DATA *worker_data/*, structure à remplir*/)
{
    if (argc != 4)
        usage(argv[0], "Nombre d'arguments incorrect");

    // remplir la structure
    *worker_data = init_worker(argv[1], argv[2], argv[3]);
    printf("++++++++++++++++++++ WORKER creation du worker %4d\n", (*worker_data)->p);

}

/************************************************************************
 * Boucle principale de traitement
 ************************************************************************/

void loop(WORKER_DATA worker_data/* paramètres */)
{
    // boucle infinie :
    while (true)
    {
        // attendre l'arrivée d'un nombre à tester
        int number = 0;
        my_read(worker_data->fdIn, &number, sizeof(int));

        // si ordre d'arrêt
        if(number == -1)
        {
            // si il y a un worker suivant, transmettre l'ordre et attendre sa fin
            if(worker_data->hasNext)
            {
                my_write(worker_data->fdOut[1], &number, sizeof(int));
                wait(NULL);  
            }
            // sortir de la boucle
            printf("------------- FIN WORKER # %3d\n", worker_data->p); 
            break;
        }
        // sinon c'est un nombre à tester, 4 possibilités :
        else
        {
            //- le nombre est premier
            if(number == worker_data->p)
            {
                //printf("++++++++++++++++++++ le nombre %4d est premier\n", number);
                my_write(worker_data->fdToMaster, &YES, sizeof(int));
            }
            else
            //- le nombre n'est pas premier
            if((number % (worker_data->p)) == 0)
            {
                //printf("++++++++++++++++++++ Le nombre %4d est divisible par moi %4d, j'envoie %4d\n",number, worker_data->p, NO);
                my_write(worker_data->fdToMaster, &NO, sizeof(int));
            }
            else
            //- s'il y a un worker suivant lui transmettre le nombre
            if (worker_data->hasNext)
            {
                my_write(worker_data->fdOut[1], &number, sizeof(int));
            }
            //- s'il n'y a pas de worker suivant, le créer
            else
            {
                worker_data->hasNext = true;
                my_pipe(worker_data->fdOut);
                int ret = my_fork();

                if (ret == 0)
                {
                    my_close(worker_data->fdOut[1]);
                    next_worker(number, worker_data->fdOut[0], worker_data->fdToMaster);
                }
                else
                {
                    //printf("++++++++++++++++++++ Worker post : %4d je cree le worker suivant %4d\n", worker_data->p, number);
                    my_close(worker_data->fdOut[0]);
                    
                }
            }
        }  
    }  
}

/************************************************************************
 * Programme principal
 ************************************************************************/

int main(int argc, char * argv[])
{
    WORKER_DATA worker_data = NULL;
    parseArgs(argc, argv, &worker_data/*, structure à remplir*/);

    // Si on est créé c'est qu'on est un nombre premier
    // Envoyer au master un message positif pour dire
    // que le nombre testé est bien premier

    my_write(worker_data->fdToMaster, &YES, sizeof(int));

    loop(worker_data/* paramètres */);

    // libérer les ressources : fermeture des files descriptors par exemple
    my_close(worker_data->fdIn);
    my_close(worker_data->fdToMaster);
    my_close(worker_data->fdOut[1]);

    free(worker_data);

    return EXIT_SUCCESS;
}
