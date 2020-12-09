#if defined HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>

#include "master_client.h"


// chaines possibles pour le premier paramètre de la ligne de commande
#define TK_STOP      "stop"
#define TK_COMPUTE   "compute"
#define TK_HOW_MANY  "howmany"
#define TK_HIGHEST   "highest"
#define TK_LOCAL     "local"


/************************************************************************
 * Usage et analyse des arguments passés en ligne de commande
 ************************************************************************/

static void usage(const char *exeName, const char *message)
{
    fprintf(stderr, "usage : %s <ordre> [<number>]\n", exeName);
    fprintf(stderr, "   ordre \"" TK_STOP  "\" : arrêt master\n");
    fprintf(stderr, "   ordre \"" TK_COMPUTE  "\" : calcul de nombre premier\n");
    fprintf(stderr, "                       <nombre> doit être fourni\n");
    fprintf(stderr, "   ordre \"" TK_HOW_MANY "\" : combien de nombres premiers calculés\n");
    fprintf(stderr, "   ordre \"" TK_HIGHEST "\" : quel est le plus grand nombre premier calculé\n");
    fprintf(stderr, "   ordre \"" TK_LOCAL  "\" : calcul de nombre premier en local\n");
    if (message != NULL)
        fprintf(stderr, "message : %s\n", message);
    exit(EXIT_FAILURE);
}

static int parseArgs(int argc, char * argv[], int *number)
{
    int order = ORDER_NONE;

    if ((argc != 2) && (argc != 3))
        usage(argv[0], "Nombre d'arguments incorrect");

    if (strcmp(argv[1], TK_STOP) == 0)
        order = ORDER_STOP;
    else if (strcmp(argv[1], TK_COMPUTE) == 0)
        order = ORDER_COMPUTE_PRIME;
    else if (strcmp(argv[1], TK_HOW_MANY) == 0)
        order = ORDER_HOW_MANY_PRIME;
    else if (strcmp(argv[1], TK_HIGHEST) == 0)
        order = ORDER_HIGHEST_PRIME;
    else if (strcmp(argv[1], TK_LOCAL) == 0)
        order = ORDER_COMPUTE_PRIME_LOCAL;
    
    if (order == ORDER_NONE)
        usage(argv[0], "ordre incorrect");
    if ((order == ORDER_STOP) && (argc != 2))
        usage(argv[0], TK_STOP" : il ne faut pas de second argument");
    if ((order == ORDER_COMPUTE_PRIME) && (argc != 3))
        usage(argv[0], TK_COMPUTE " : il faut le second argument");
    if ((order == ORDER_HOW_MANY_PRIME) && (argc != 2))
        usage(argv[0], TK_HOW_MANY" : il ne faut pas de second argument");
    if ((order == ORDER_HIGHEST_PRIME) && (argc != 2))
        usage(argv[0], TK_HIGHEST " : il ne faut pas de second argument");
    if ((order == ORDER_COMPUTE_PRIME_LOCAL) && (argc != 3))
        usage(argv[0], TK_LOCAL " : il faut le second argument");
    if ((order == ORDER_COMPUTE_PRIME) || (order == ORDER_COMPUTE_PRIME_LOCAL))
    {
        *number = strtol(argv[2], NULL, 10);
        if (*number < 2)
             usage(argv[0], "le nombre doit être >= 2");
    }       
    
    return order;
}


/************************************************************************
 * Fonction principale
 ************************************************************************/

int main(int argc, char * argv[])
{
    int number = 0;
    int order = parseArgs(argc, argv, &number);
    printf("%d\n", order); // pour éviter le warning

    // order peut valoir 5 valeurs (cf. master_client.h) :
    //      - ORDER_COMPUTE_PRIME_LOCAL
    //      - ORDER_STOP
    //      - ORDER_COMPUTE_PRIME
    //      - ORDER_HOW_MANY_PRIME
    //      - ORDER_HIGHEST_PRIME
    //
    // si c'est ORDER_COMPUTE_PRIME_LOCAL
    if(order == ORDER_COMPUTE_PRIME_LOCAL)
    {
    // alors c'est un code complètement à part multi-thread
        is_prime_local(number);    
    }
    // sinon
    else
    {
    //- entrer en section critique :
    //       . pour empêcher que 2 clients communiquent simultanément
    //       . le mutex est déjà créé par le master

        int sid_1 = my_semget_key(MON_FICHIER, PROJ_ID_1, 1, 0);

        // prendre
        prendre(sid_1);

        int sid_2 = my_semget_key(MON_FICHIER, PROJ_ID_2, 1, 0);

    //    - ouvrir les tubes nommés (ils sont déjà créés par le master)
    //           . les ouvertures sont bloquantes, il faut s'assurer que
    //             le master ouvre les tubes dans le même ordre

        int client_master = my_open(TCM, O_WRONLY);
        int master_client = my_open(TMC, O_RDONLY);

    //    - envoyer l'ordre et les données éventuelles au master
        my_write(client_master, &order, sizeof(int));
        if(order == ORDER_COMPUTE_PRIME)
        {
            my_write(client_master, &number, sizeof(int));        

        }

    //    - attendre la réponse sur le second tube
        int reponse;
        my_read(master_client, &reponse, sizeof(int));

        if(order == ORDER_COMPUTE_PRIME)
            if(reponse == 1)
                printf("Le resultat : %d est premier\n", number);
            else
                printf("Le resultat : %d n'est pas premier\n", number);
        else
            if(order == ORDER_STOP && reponse == ORDER_STOP)
                printf("accusé de réception reçu, aurevoir\n");
            else
                printf("Resultat : %d\n", reponse);

    //    - sortir de la section critique
        // vendre
        vendre(sid_1);

    //    - libérer les ressources (fermeture des tubes, ...)
        my_close(master_client);
        my_close(client_master);
    //    - débloquer le master grâce à un second sémaphore (cf. ci-dessous)
        printf("Jai FINI...\n");
        // vendre
        vendre(sid_2);

    }
    // 
    // Une fois que le master a envoyé la réponse au client, il se bloque
    // sur un sémaphore ; le dernier point permet donc au master de continuer
    //
    // N'hésitez pas à faire des fonctions annexes ; si la fonction main
    // ne dépassait pas une trentaine de lignes, ce serait bien.
    
    return EXIT_SUCCESS;
}
