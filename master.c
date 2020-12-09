#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>


#include "myassert.h"

#include "master_client.h"
#include "master_worker.h"


/************************************************************************
 * Données persistantes d'un master
 ************************************************************************/

// on peut ici définir une structure stockant tout ce dont le master
// a besoin

typedef struct data
{
    int nombre_calcules;
    int plus_grand_nombre_teste;
    int plus_grand_nombre_premier;
    int fdOut[2];
    int fdToMaster[2];

}*MASTER_DATA;


// fonction init_master
MASTER_DATA init_master(){
    MASTER_DATA master_data = malloc(sizeof(struct data));
    master_data->nombre_calcules = 1;
    master_data->plus_grand_nombre_teste = 2;
    master_data->plus_grand_nombre_premier = 2;
    return master_data;
}


void worker(MASTER_DATA master_data)
{
    printf("    Lancement du worker 2 ...\n");

    my_close(master_data->fdOut[1]); // fermer l'ecriture du tube fdOut du precedent (on ne fait que recevoir)
    my_close(master_data->fdToMaster[0]); // fermer la lecture du tube fdToMaster (on envoie les reponses dedans)

    next_worker(2, master_data->fdOut[0], master_data->fdToMaster[1]);
}

void master(MASTER_DATA master_data){

    my_close(master_data->fdOut[0]); // fermer la lecture du tube fdOut (on envoie les order et les donnees dedans)
    my_close(master_data->fdToMaster[1]); // fermer l'ecriture du tube fdToMaster (on ne fait que recevoir)

    int reponse = -2;
    my_read(master_data->fdToMaster[0], &reponse, sizeof(int));
    if(reponse == 1)
        printf("Mon worker est pret\n");
    else
        printf("mon worker a un probleme\n");
    
}
/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s\n", exeName);
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}


/************************************************************************
 * boucle principale de communication avec le client
 ************************************************************************/
void loop(MASTER_DATA master_data, int sid/* paramètres */)
{
    // boucle infinie :
    while (true)
    {
    
    // - ouverture des tubes (cf. rq client.c)
        printf("\nMASTER : J'attends des clients ...\n\n");

        int client_master = my_open(TCM, O_RDONLY);
        int master_client = my_open(TMC, O_WRONLY);

        printf("MASTER : AH ! j'ai un client ...\n\n");


    // - attente d'un ordre du client (via le tube nommé)
        int order;
        my_read(client_master, &order, sizeof(int));

    // - si ORDER_STOP
        if(order == ORDER_STOP)
        {
            // . envoyer ordre de fin au premier worker et attendre sa fin
            printf("MASTER : J'attends la fin de mon worker...\n");
            my_write(master_data->fdOut[1], &order, sizeof(int));
            wait(NULL);

            // . envoyer un accusé de réception au client
            my_write(master_client, &order, sizeof(int));
            prendre(sid);
            printf("\nMASTER : FINI !\n");
            break;
        } 
        else

    // - si ORDER_COMPUTE_PRIME
        if(order == ORDER_COMPUTE_PRIME)
        {
            // . récupérer le nombre N à tester provenant du client
            int number;
            my_read(client_master, &number, sizeof(int));
            int M = master_data->plus_grand_nombre_teste;
            int reponse;
            // . construire le pipeline jusqu'au nombre N-1 (si non encore fait) :
            //       il faut connaître le plus nombre (M) déjà envoyé aux workers
            if(number > M)
            {
            //  on leur envoie tous les nombres entre M+1 et N-1
                for (int i = M+1; i < number; i++)
                {
                    my_write(master_data->fdOut[1], &i, sizeof(int));
                    //note : chaque envoie déclenche une réponse des workers
                    my_read(master_data->fdToMaster[0], &reponse, sizeof(int));

                    if(reponse == 1)
                    {
                        master_data->nombre_calcules++;
                        master_data->plus_grand_nombre_premier = i;
                    }
                }
                
                master_data->plus_grand_nombre_teste = number;
            }
            // . envoyer N dans le pipeline
            my_write(master_data->fdOut[1], &number, sizeof(int));
            // . récupérer la réponse
            my_read(master_data->fdToMaster[0], &reponse, sizeof(int));
            if(reponse == 1 && M < number)
            {
                master_data->nombre_calcules++;
                master_data->plus_grand_nombre_premier = number;
            }

            printf("\nMASTER : C'est bon, j'envoi le resultat : %d ...\n", reponse);

            // . la transmettre au client
            my_write(master_client, &reponse, sizeof(int));

            // MAJ du plus grand nombre testé
            if(number > master_data->plus_grand_nombre_teste)
                master_data->plus_grand_nombre_teste = number;

        }
        else
        // - si ORDER_HOW_MANY_PRIME
        if(order == ORDER_HOW_MANY_PRIME)
        {
            // . transmettre la réponse au client
            my_write(master_client, &(master_data->nombre_calcules), sizeof(int));

        }
        else
        // - si ORDER_HIGHEST_PRIME
        if(order == ORDER_HIGHEST_PRIME)
        {
            //. transmettre la réponse au client
            my_write(master_client, &(master_data->plus_grand_nombre_premier), sizeof(int));

        }
    // - fermer les tubes nommés
        my_close(client_master);
        my_close(master_client);
    // - attendre ordre du client avant de continuer (sémaphore : précédence)
        prendre(sid);

    // - revenir en début de boucle
    }
    //
    // il est important d'ouvrir et fermer les tubes nommés à chaque itération
    // voyez-vous pourquoi ?
    // pour que le master puisse se connecter a un autre client
}


/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
    if (argc != 1)
        usage(argv[0], NULL);

    MASTER_DATA master_data = init_master();

    // - création des sémaphores
    int sid_1 = my_semget_key(MON_FICHIER, PROJ_ID_1, 1, IPC_CREAT | 0641);
    int sid_2 = my_semget_key(MON_FICHIER, PROJ_ID_2, 1, IPC_CREAT | 0641);

    // initialisation
    my_semctl(sid_1, 1);
    my_semctl(sid_2, 0);

    // - création des tubes nommés
    my_mkfifo(TCM);
    my_mkfifo(TMC);
    
    // - création du premier worker
    
    // creation des tubes
    my_pipe(master_data->fdOut);
    my_pipe(master_data->fdToMaster);

    int ret = my_fork();
    if(ret == 0)
    {
        worker(master_data);
    }
    else
    {
        master(master_data);
    }
    
    // boucle infinie
    loop(master_data, sid_2/* paramètres */);

    // destruction des tubes nommés, des sémaphores, ...
    my_unlink(TMC);
    my_unlink(TCM);

    my_semdelete(sid_1);
    my_semdelete(sid_2);

    free(master_data);

    return EXIT_SUCCESS;
}

// N'hésitez pas à faire des fonctions annexes ; si les fonctions main
// et loop pouvaient être "courtes", ce serait bien
