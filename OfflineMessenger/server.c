#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/stat.h>


/* portul folosit */
#define PORT 2908

/* codul de eroare returnat de anumite apeluri */
extern int errno;

typedef struct thData{
    int idThread; //id-ul thread-ului tinut in evidenta de acest program
    int cl; //descriptorul intors de accept
}thData;

static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
 void raspunde(void *);
 struct client_date{
    char nume[50];
    int adresa;
}client_date[100];
int nr_clienti=0;
int main ()
{
    struct sockaddr_in server;	// structura folosita de server
    struct sockaddr_in from;

    int sd;		//descriptorul de socket
    int pid;
    pthread_t th[100];    //Identificatorii thread-urilor care se vor crea
    int i=0;


    /* crearea unui socket */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("[server]Eroare la socket().\n");
        return errno;
    }
    /* utilizarea optiunii SO_REUSEADDR */
    int on=1;
    setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));

    /* pregatirea structurilor de date */
    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));

    /* umplem structura folosita de server */
    /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    /* utilizam un port utilizator */
    server.sin_port = htons (PORT);

    /* atasam socketul */
    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
        perror ("[server]Eroare la bind().\n");
        return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen (sd, 2) == -1)
    {
        perror ("[server]Eroare la listen().\n");
        return errno;
    }

    /* servim in mod concurent clientii...folosind thread-uri */
    while (1)
    {
        int client;
        thData * td; //parametru functia executata de thread
        int length = sizeof (from);

        printf ("[server]Asteptam la portul %d...\n",PORT);
        fflush (stdout);

        // client= malloc(sizeof(int));
        /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
        if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
        {
            perror ("[server]Eroare la accept().\n");
            continue;
        }

        /* s-a realizat conexiunea, se astepta mesajul */

        // int idThread; //id-ul threadului
        // int cl; //descriptorul intors de accept

        td=(struct thData*)malloc(sizeof(struct thData));
        td->idThread=i++;
        td->cl=client;

        pthread_create(&th[i], NULL, &treat, td);

    }//while
};
static void *treat(void * arg)
{
        int valid=0;

        struct thData tdL;
        tdL = *((struct thData *) arg);
        printf("[thread]- %d - Asteptam mesajul...\n", tdL.idThread);
        fflush(stdout);
        pthread_detach(pthread_self());
        raspunde((struct thData *) arg);

        /* am terminat cu acest client, inchidem conexiunea */
        close((intptr_t) arg);
        return (NULL);


};
int logare(char comanda[100],int cl,char par[100]){
    FILE *fisi,*p;
    char nume[20]="";
    char parola[20]="";
    int date_gasite=0;
    int lin_nume=0,lin_parola=0,linie=0;
    strcpy(comanda,comanda+6);//prelucrare comanda
    comanda[strlen(comanda)-1]='\0';
    if((fisi=fopen("date.txt","r"))==NULL)
        {
        printf("eraore date\n");
        exit(EXIT_FAILURE);
        }
    else
    {
        while(fgets(nume,100,fisi) && date_gasite==0) {   //confirmarea corectitudinii datelor de logare
            nume[strlen(nume)-1]='\0';
            lin_nume++;
            if (strcmp(nume, comanda) == 0) {
                date_gasite=1;
                linie=lin_nume;
                   // returneaza -2 daca numele exista dar este parola gresita
            }
        }
        fclose(fisi);
        if((p=fopen("parole.txt","r"))==NULL)
        {
        printf("eraore date\n");
        exit(EXIT_FAILURE);
        }
        else{
            while(fgets(parola,100,p)&&lin_parola<linie){
                lin_parola++;
                if(lin_parola==linie)
                    if(strcmp(par,parola)!=0)
                        return -2;
            }
        }

    }
    if(date_gasite==0)
        return 3;     //returneaza 3 daca nu a fost gasit numele de utilizator in fisierul cu date
    for(int i=0;i<nr_clienti;i++)
        if(strcmp(client_date[i].nume,comanda)==0)
            return 0;   // returneaza 0 daca exista deja cineva conectat cu acest nume
            // daca nu a returnat nimic pana acum inseamna ca datele introduse sunt corecte deci ii face logarea si returneaza 1
    strcpy(client_date[nr_clienti].nume,comanda);
    client_date[nr_clienti].adresa=cl;
    nr_clienti++;
   /* for(int i=0;i<nr_clienti;i++)
        printf("-> %s <-   >> %d <<\n",client_date[i].nume,client_date[i].adresa);*/
    return 1;
}

void write_to(char comanda[100],char mesaj[100],int client_curent)
{
    int aici=-1,client_adrs;
    char mesaj_complet[1000]="";
    char nume_client_curent[100]="";
    char nume_fisier[100]="";
    char nume_fisier_mesaj[100]="";
    char haida[100]="";
    FILE *fisi,*fisi_tempo,*fisi_tempo2, *fisi_nrmesaj;

    strcpy(comanda,comanda+9);//prelucrare comanda
    comanda[strlen(comanda)-1]='\0';

    for(int i=0;i<nr_clienti;i++) {  //aflarea adresei, deci numele clientului curent care trimite mesajul
        if (strcmp(client_date[i].nume, comanda) == 0)
            aici = client_date[i].adresa;
        if(client_date[i].adresa==client_curent)
            strcpy(nume_client_curent, client_date[i].nume);

    }
        if(strcmp(comanda,nume_client_curent)>0){ //formarea celor doua fisiere pentru conversatia celor doi clienti alfabetic
            strcat(nume_fisier,comanda);
            strcat(nume_fisier,nume_client_curent);
            strcat(nume_fisier_mesaj,comanda);
            strcat(nume_fisier_mesaj,nume_client_curent);
        }
        else{
            strcat(nume_fisier,nume_client_curent);
            strcat(nume_fisier,comanda);
            strcat(nume_fisier_mesaj,nume_client_curent);
            strcat(nume_fisier_mesaj,comanda);
        }
        strcat(mesaj_complet, nume_client_curent);// formarea inceputului de mesaj
        strcat(mesaj_complet, "~");
        strcat(mesaj_complet, mesaj);

        int count=0,a=0;
        char nr[20]="",c;
        char nume_tempo2[100]="";

        strcat(nume_fisier_mesaj,"nr"); //fisierul cu nr mesajului
        strcat(nume_tempo2,comanda);//fisierul temporar pentru atunci cand clientul-dest nu este conectat
        strcat(nume_tempo2,"_tempo");
        //printf("mesajul este : %s\n", mesaj_complet);
    if((fisi_nrmesaj=fopen(nume_fisier_mesaj,"a+"))==NULL) //daca e primul mesaj se creaza fisierul
    {
        printf("unable to create file\n");
        exit(EXIT_FAILURE);
    }
    if(fgets(nr,10,fisi_nrmesaj)) //se incrementeaza nr ultimului mesaj
    {
        //printf("nr=%s\n",nr);
        a=atoi(nr);
        fclose(fisi_nrmesaj);
        remove((nume_fisier_mesaj));
        if((fisi_nrmesaj=fopen(nume_fisier_mesaj,"a+"))==NULL)
        {
            printf("unable to create file\n");
            exit(EXIT_FAILURE);
        }
       //printf("%d\n",a);
       a++;
       //printf("---%s--\n",nr_incrementat);
      fprintf(fisi_nrmesaj,"%d",a);
      fclose(fisi_nrmesaj);
    }
    else //primul mesaj se scrie 1 in fisier
    {
        fputs("1",fisi_nrmesaj);
        fclose(fisi_nrmesaj);
    }
        if(aici==-1) //daca clientul-dest nu este conectat, nu s-a gasit adresa sa deci aici a ramas -1, deci se creeaza un fisier temporar
        {             // unde se scriu mesajele care ii vor aparea direct atunci cand se va conecta
            if((fisi_tempo2=fopen(nume_tempo2,"a+"))==NULL)
            {
                printf("unable to create file\n");
                exit(EXIT_FAILURE);
            }
            fputs(mesaj_complet,fisi_tempo2);
            fclose(fisi_tempo2);
        }
        if((fisi_tempo=fopen(comanda,"a+"))==NULL)  //se deschide fisierul care va contine mesajele pe care acesta le primeste intre timp de la toti utilizatorii
        {
            printf("unable to create file\n");
            exit(EXIT_FAILURE);
        }
        fputs(mesaj_complet,fisi_tempo);
        fclose(fisi_tempo);
        if((fisi=fopen(nume_fisier,"a+"))==NULL) //se trece mesajul prelucrat in istoricul conversatiei celor doi
            {
                printf("unable to create file\n");
                exit(EXIT_FAILURE);
            }
            printf("[Thread %s]Mesajul a fost trasmis cu succes:::::::.\n", mesaj);
            // se prelucreaza putin nr
            count=a;
            if(count<2)
                count=1;
            //printf("%d\n",count);
            int copie=count;
            char asta_e_ultimul[150]="",b[10]="";

            count=0;
             while(copie!=0){
                 count=(count*10)+copie%10;
                 copie=copie/10;
             }
             //printf("23%d23\n",count);
             while(count!=0){
                 b[strlen(b)]=(count%10)+48;
                 count=count/10;
             }
             strcat(asta_e_ultimul,b);
             strcat(asta_e_ultimul,") ");
             strcat(asta_e_ultimul,mesaj_complet);
             fputs(asta_e_ultimul,fisi);
             fclose(fisi);

        if (write(client_curent, "mesajul a fost trimis ! ^^ ", 100) <= 0) {
            // printf("[Thread %d] ", aici);
            perror("[Thread]Eroare la write() catre client.\n");
        }
}

void reply_to(char mesaj[100],char comanda[100],int client_curent,char catre[100]) {

    int aici=-1,a=0;
    char id[10]="";
    char mesaj_complet[1000]="";
    char nume_client_curent[100]="";
    char nume_tempo2[100]="";
    FILE *fisi,*fisi_tempo,*fisi_tempo2,*fisi_nrmesaj;

    strcpy(comanda,comanda+7); //prelucrare comanda
    comanda[strlen(comanda)-1]='\0';
    //printf("reply modee %s\n",comanda);

    strcat(nume_tempo2,catre);
    strcat(nume_tempo2,"_tempo");
    for(int i=0;i<nr_clienti;i++) {   //obtinerea adresei in cazul in care clientul-dest este conectat ( memorata in 'aici')
        if (strcmp(client_date[i].nume, comanda) == 0)
            aici = client_date[i].adresa;
        if(client_date[i].adresa==client_curent)
            strcpy(nume_client_curent,client_date[i].nume);
    }
    strcat(mesaj_complet,nume_client_curent);
    //printf("mesaj complet--->>> %s\n",mesaj_complet);
    strcat(mesaj_complet," replied with: ");
    //printf("mesaj complet--->>> %s\n",mesaj_complet);
    strcat(mesaj_complet,mesaj);
    //printf("mesaj complet--->>> %s\n",mesaj_complet);
    strcat(mesaj_complet, " at ");
    //printf("mesaj complet--->>> %s\n",mesaj_complet);
    strcat(mesaj_complet,comanda);
    strcat(mesaj_complet,"\n");
    //printf("mesaj complet--->>> %s\n",mesaj_complet);


    int count=0;
    char nume_fisier[100]="",c;
    char mesaj_SEMI[100]="";
    char nume_fisier_mesaj[100]="";

    if(strcmp(catre,nume_client_curent)>0){ // se formeaza fisierul cu istoricul celor doi pentru a trece mesajul
        strcat(nume_fisier,catre);
        strcat(nume_fisier,nume_client_curent);
        strcat(nume_fisier_mesaj,catre);
        strcat(nume_fisier_mesaj,nume_client_curent);
    }
    else{
        strcat(nume_fisier,nume_client_curent);
        strcat(nume_fisier,catre);
        strcat(nume_fisier_mesaj,nume_client_curent);
        strcat(nume_fisier_mesaj,catre);
    }
    strcat(mesaj_SEMI, nume_client_curent);
    strcat(mesaj_SEMI, "~");
    strcat(mesaj_SEMI, mesaj_complet);

    strcat(nume_fisier_mesaj,"nr");
    if((fisi_nrmesaj=fopen(nume_fisier_mesaj,"a+"))==NULL)//se incrementeaza nr mesajului
    {
        printf("unable to create file\n");
        exit(EXIT_FAILURE);
    }
    char nr[10]="";
    if(fgets(nr,10,fisi_nrmesaj))
    {
        printf("nr=%s\n",nr);

        a=atoi(nr);
        fclose(fisi_nrmesaj);
        remove((nume_fisier_mesaj));
        a++;

    }
    else
    {
        fputs("1",fisi_nrmesaj);
        //printf("zero\n");
        fclose(fisi_nrmesaj);

    }
    if(a>atoi(comanda)) { //verifica validitatea nr indicat la comanda de reply
        if((fisi_nrmesaj=fopen(nume_fisier_mesaj,"a+"))==NULL)
        {
            printf("unable to create file\n");
            exit(EXIT_FAILURE);
        }
        //printf("%d\n",a);
        //printf("---%s--\n",nr_incrementat);
        fprintf(fisi_nrmesaj,"%d",a);
        fclose(fisi_nrmesaj);
        if (aici ==-1) { //se verifica daca clientul-dest e conectat iar in caz ca nu e se scriu mesajele in fiserul temporar
            if ((fisi_tempo2 = fopen(nume_tempo2, "a+")) == NULL) {
                printf("unable to create file\n");
                exit(EXIT_FAILURE);
            }
            fputs(mesaj_complet, fisi_tempo2);
            fclose(fisi_tempo2);
        }

        if ((fisi_tempo = fopen(catre, "a+")) ==
            NULL) //se trece mesajul si in fisierul cu mesajele de la ceilalti utilizator, ale clientului-dest primite intre timp
        {
            printf("unable to create file\n");
            exit(EXIT_FAILURE);
        }
        fputs(mesaj_complet, fisi_tempo);
        fclose(fisi_tempo);
        if ((fisi = fopen(nume_fisier, "a+")) == NULL) //se trece mesajul final
        {
            printf("unable to create file\n");
            exit(EXIT_FAILURE);
        }
        printf("[Thread %s]Mesajul a fost trasmis cu succes:::::::.\n", mesaj);
        //printf("se pare ca aici\n");
        count = a;
        if (count < 2)
            count = 1;
        //printf("%d\n",count);
        int copie = count;
        char asta_e_ultimul[150] = "", b[10] = "";
        count = 0;
        while (copie != 0) {
            count = (count * 10) + copie % 10;
            copie = copie / 10;
        }
        while (count != 0) {
            b[strlen(b)] = (count % 10) + 48;
            //printf("%c\n",b[strlen(b)]);
            count = count / 10;
        }
        strcat(asta_e_ultimul, b);
        //printf("-------%d---------%s\n",count,asta_e_ultimul);
        strcat(asta_e_ultimul, ") ");
        strcat(asta_e_ultimul, mesaj_SEMI);
        fputs(asta_e_ultimul, fisi);
        //printf("aici\n");
        fclose(fisi);
        if (write(client_curent, "mesajul a fost trimis !! ", 100) <= 0) {
            // printf("[Thread %d] ", aici);
            perror("[Thread]Eroare la write() catre client.\n");
        }
    }
    else { a--;
        if((fisi_nrmesaj=fopen(nume_fisier_mesaj,"a+"))==NULL)
        {
            printf("unable to create file\n");
            exit(EXIT_FAILURE);
        }
        printf("%d\n",a);


        //printf("---%s--\n",nr_incrementat);
        fprintf(fisi_nrmesaj,"%d",a);

        fclose(fisi_nrmesaj);
        if (write(client_curent, "id-ul mesajului indicat nu este corect !! ", 100) <= 0) {
            // printf("[Thread %d] ", aici);
            perror("[Thread]Eroare la write() catre client.\n");
        }
    }
}

 void raspunde(void *arg)
{ while(1) {
        int nr = 0, i = 0;
        char comanda[100]="";
        struct thData tdL;
        tdL = *((struct thData *) arg);

        if (read(tdL.cl, comanda, 100) <= 0) {
            printf("[Thread %d]\n", tdL.idThread);
            perror("Eroare la read() de la client.\n");
            break;

        }
        printf("[Thread %d]Mesajul a fost receptionat...%d\n", tdL.idThread, nr);
        printf("comanda primita: %s\n", comanda);
        /*pregatim mesajul de raspuns */
       /* if (strcmp(comanda, "quit()") == 0)
            break;*/

       ///verificam logarea
        if(strstr(comanda,"login:")) {
            char parola[20]="";

            if (read(tdL.cl, parola, 20) <= 0) {
                printf("[Thread %d]\n", tdL.idThread);
                perror("Eroare la citire parola.\n");

            }
            int verif=0;
            verif=logare(comanda, tdL.cl,parola);
            if ( verif== 0) {
                write(tdL.cl, "userul este deja logat", 100);
            } else  if (verif ==3)
                write(tdL.cl, "nume de utilizator gresit", 100);
            else  if (verif == -2)
                write(tdL.cl, "parola gresita", 100);

            else {
                FILE *verificare_tempo;
                char nume_tempo2[100]="";
                strcat(nume_tempo2,client_date[tdL.idThread].nume);
                strcat(nume_tempo2,"_tempo");

                if (verificare_tempo = fopen(nume_tempo2, "r")) {
                    if (write(tdL.cl, "user connected !\n You have new messages received when you were not logged in : ", 100) <= 0) {
                        printf("[Thread %d] ", tdL.idThread);
                        perror("[Thread]Eroare la write() catre client.\n");
                    }
                    char mesaje_noi[100] = "";
                    while (fgets(mesaje_noi, 100, verificare_tempo) != NULL) {

                        write(tdL.cl, mesaje_noi, 100);
                    }
                    if (write(tdL.cl, "gata", 100) <= 0)
                        perror(" client.\n");

                    //printf("daa\n");
                    fclose(verificare_tempo);
                    remove(nume_tempo2);
                } else {
                    if (write(tdL.cl, "user connected !", 100) <= 0) {
                        printf("[Thread %d] ", tdL.idThread);
                        perror("[Thread]Eroare la write() catre client.\n");
                    }
                }

            }
        }
        else if(strstr(comanda,"archive_with:"))
        {
            strcpy(comanda,comanda+13);
            comanda[strlen(comanda)-1]='\0';

            char nume_client_curent[100]="";
            char nume_fis_hist[100]="";
            FILE *hist;
            for(int i=0;i<nr_clienti;i++)
                if(client_date[i].adresa==tdL.cl) {
                    strcpy(nume_client_curent, client_date[i].nume);
                    i=nr_clienti;
                }
            printf("%s\n",nume_client_curent);
            if(strcmp(comanda,nume_client_curent)>0) {
                strcat(nume_fis_hist,comanda);
                strcat(nume_fis_hist, nume_client_curent);
            }
            else {
                strcat(nume_fis_hist,nume_client_curent);
                strcat(nume_fis_hist, comanda);
            }
            printf("%s\n",nume_fis_hist);
            char istoric_msg[100]="";
            if(hist=fopen(nume_fis_hist,"r"))
            {
                while (fgets(istoric_msg, 100, hist) != NULL) {

                    write(tdL.cl, istoric_msg, 100);
                }
                write(tdL.cl,"gata",100);
                fclose(hist);

            }
            else
            {
                write(tdL.cl, "EMPTY CONVERSATION ! \n", 100);
            write(tdL.cl,"gata",100);
            }
            char catre[100]="";
            strcpy(catre,comanda);
            bzero(comanda,100);

            if (read(tdL.cl, comanda, 100) <= 0) {
                printf("[Thread %d]\n", tdL.idThread);
                perror("Eroare la read() de la client.\n");

            }
            if(strstr(comanda,"reply->"))
            {
                char mesaj[100]="";
                if (read(tdL.cl, mesaj, 100) <= 0)
                {
                    perror("eroare la server client transmisie mesaj\n");
                    exit(1);
                }
                reply_to(mesaj,comanda,tdL.cl,catre);
            }
            //if(strstr(comanda,"back"))
                //write(tdL.cl,"back",100);

        }
        else if(strstr(comanda,"write_to:")) {
            char mesaj[100]="";
            if (read(tdL.cl, mesaj, 100) <= 0)
            {
                perror("eroare la server client transmisie mesaj\n");
                exit(1);
            }
            write_to(comanda,mesaj,tdL.cl);
        }
        else if(strstr(comanda,"latest messages"))
         {
             FILE*fis;
             char nume[100]="";
             for(int i=0;i<nr_clienti;i++)
                 if(client_date[i].adresa==tdL.cl)
                 {strcpy(nume,client_date[i].nume);
                     i=nr_clienti+1;}


             if(write(tdL.cl,nume,100)<=0)
             {
                 perror("eroare la server client transmisie mesaj\n");
                 exit(1);
             }

         }
         else if(strstr(comanda,"logout"))
         {
             for(int i=0;i<nr_clienti;i++)
                 if(client_date[i].adresa==tdL.cl) {
                     for (int j = i; j < nr_clienti - 1; j++) {
                         client_date[j] = client_date[j + 1];
                     }
                     i=nr_clienti+3;
                     nr_clienti--;

                 }
             //for(int i=0;i<nr_clienti;i++)
                 //printf("-> %s <-   >> %d <<\n",client_date[i].nume,client_date[i].adresa);
         }
    }
}
