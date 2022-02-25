#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

int main (int argc, char *argv[]) {
    int sd;            // descriptorul de socket
    struct sockaddr_in server;    // structura folosita pentru conectare
    /* exista toate argumentele in linia de comanda? */
    if (argc != 3) {
        printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }
    /* stabilim portul */
    port = atoi(argv[2]);

    /* cream socketul */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Eroare la socket().\n");
        return errno;
    }
    /* umplem structura folosita pentru realizarea conexiunii cu serverul */
    /* familia socket-ului */
    server.sin_family = AF_INET;
    /* adresa IP a serverului */
    server.sin_addr.s_addr = inet_addr(argv[1]);
    /* portul de conectare */
    server.sin_port = htons(port);
    /* ne conectam la server */
    if (connect(sd, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1) {
        perror("[client]Eroare la connect().\n");
        return errno;
    }
    //comunicarea cu serverul
    int recent=1;
    char comanda[100]="";
    while (1)
    {
        bzero(comanda,100);
    printf("[client]Introduceti o comanda din: \n \n");
    if(recent==1)
        printf("Please enter your username to log in:  >> login:username \n >> ");
    else
        printf(" 1. write_to:...\n 2. archive_with:... \n 3. latest messages \n 4. logout \n\n >> ");

    fflush(stdout);
    read(0, comanda, 100);

    /* trimiterea mesajului la server */

        char mesaj[100]="";
    if(strstr(comanda,"login:")) {
        if (recent == 0) {
            printf("you need to log out first! \n");
        } else {
            if (write(sd, comanda, 100) <= 0) {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
            }
            char parola[20]="";
            printf("Dati parola:\n >> ");
            fflush(stdout);
            read(0, parola, 100);
            if (write(sd, parola, 100) <= 0) {
                perror("[client]Eroare la write() parola spre server.\n");
                return errno;
            }
            char raspuns[100] = "";
            /* citirea raspunsului dat de server
               (apel blocant pina cind serverul raspunde) */
            if (read(sd, raspuns, 100) < 0) {
                perror("[client]Eroare la read() de la server.\n");
                return errno;
            }

            /* afisam mesajul primit */
            printf("[client]Mesajul primit este: %s\n", raspuns);
            int ok2 = 0;
            if (strstr(raspuns, "user connected !")) {
                if (strstr(raspuns, "You have new messages received when you were not logged in")) {
                    printf(".....................................\n\n");
                    while (read(sd, raspuns, 100) > 0 && ok2 == 0) {
                        if (strstr(raspuns, "gata")) {
                            ok2 = 1;
                            break;
                        }
                        printf("%s", raspuns);
                    }
                    printf("\n.....................................\n\n");
                }
                else printf(" no messages received when you were not logged in \n");
                recent = 0;
            }
        }
    }
       if(strstr(comanda, "archive_with:")){
           if (write(sd, comanda, 100) <= 0) {
               perror("[client]Eroare la write() spre server.\n");
               return errno;
           }
           char raspuns[100]="";
           int ok3=0;
           printf("\n..................................\n\n");
           while(read(sd,raspuns,100)>0 && ok3==0) {

               if(strstr(raspuns,"gata")) {
                   ok3 = 1;
                   break;
               }
               printf("%s", raspuns);
           }
           char comanda[100]="";
           printf("\n..................................\n\n");
           printf("[client]Introduceti o comanda din: \n \n");
           printf(" 1. reply-><nr> \n 2. back \n >>");
           fflush(stdout);
           int verif=0;
           while(verif==0) {
               read(0, comanda, 100);
               if (strstr(comanda, "back")) {
                   if (write(sd, comanda, 100) <= 0) {
                       perror("[client]Eroare la write() spre server.\n");
                       return errno;
                   }
                   verif=1;
               }

               if (strstr(comanda, "reply->")) {
                    verif=1;
                   if (write(sd, comanda, 100) <= 0) {
                       perror("[client]Eroare la write() spre server.\n");
                       return errno;
                   } else
                       printf("[client]write the mesagge...\n ");
                   fflush(stdout);
                   mesaj[0] = '\0';
                   if (read(0, mesaj, 100) <= 0) {
                       perror("eroare la citire mesaj.\n");
                       return errno;
                   }

                   // printf("client----mesaj===%s\n",mesaj);
                   fflush(stdout);
                   if (write(sd, mesaj, 100) <= 0) {
                       perror("[client]Eroare la write() spre server.\n");
                       return errno;
                   }
                   char raspuns[100] = "";
                   /* citirea raspunsului dat de server
                      (apel blocant pina cind serverul raspunde) */
                   if (read(sd, raspuns, 100) < 0) {
                       perror("[client]Eroare la read() de la server.\n");
                       return errno;
                   }

                   /* afisam mesajul primit */
                   printf("[client]Mesajul primit este: %s\n", raspuns);
                   //if(strstr(raspuns,"id-ul mesajului indicat nu este corect !!"))
                     //  verif=0;
               }
               if(verif==0)
               {printf("comanda gresita !\n >>");
                   fflush(stdout);
           }

       }
       }

        if(strstr(comanda,"write_to:")) {
            if (write(sd, comanda, 100) <= 0) {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
            }
            printf("[client]write the mesagge...\n ");
            fflush(stdout);
            if(read(0, mesaj, 100)<=0)
            {
                perror("eroare la citire mesaj.\n");
                return errno;
            }
           // printf("client----mesaj===%s\n",mesaj);
            fflush(stdout);
            if (write(sd, mesaj, 100) <= 0) {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
            }
            char raspuns[100]="";
            /* citirea raspunsului dat de server
               (apel blocant pina cind serverul raspunde) */
            if (read(sd, raspuns, 100) < 0) {
                perror("[client]Eroare la read() de la server.\n");
                return errno;
            }

            /* afisam mesajul primit */
            printf("[client]Mesajul primit este: %s\n", raspuns);
        }

        if(strstr(comanda,"latest messages"))
        { if (write(sd, comanda, 100) <= 0) {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
            }
            char mesaje[100]="";
            FILE*fis;
            char raspuns[100]="";
            // citirea raspunsului dat de server
               //(apel blocant pina cind serverul raspunde)
            if (read(sd, raspuns, 100) < 0) {
                perror("[client]Eroare la read() de la server.\n");
                return errno;
            }
            printf("\n");
            if(fis=fopen(raspuns,"r")) {

                while (fgets(mesaje, 100, fis) != NULL)
                    printf("%s", mesaje);
                printf("\n");
            }
            else
                printf(" No messages ! \n");

        }
        if(strstr(comanda,"logout"))
        {
            if (write(sd, comanda, 100) <= 0) {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
            }
            recent=1;
        }





    /* inchidem conexiunea, am terminat */
}
    close (sd);
}
