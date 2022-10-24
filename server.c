#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <utmp.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <time.h>
#define read_end 0
#define write_end 1
#define PARENT 0
#define CHILD 1
int main(int argc, char *argv[])
{
    
    bool ok = false;
    int fd_from_client, fd_to_client;
    if (mkfifo("myfifo2.txt", 0777) == -1)
    {
        if (errno != EEXIST)
        {
            printf("Imposibil de creat fisierul fifo2\n");
            return 1;
        }
    }
    do
    {
        char arr2[100]="";
        char raspuns[100]="";
        fd_from_client = open("myfifo1.txt", O_RDONLY);
        if (fd_from_client == -1)
        {
            return 2;
        }
        fd_to_client = open("myfifo2.txt", O_WRONLY);
        if (fd_to_client == -1)
        {
            return 4;
        }
        read(fd_from_client, arr2, sizeof(char) * 99);
        if (fd_from_client == -1)
        {
            return 3;
        }
        printf("%s", arr2);
        // if (strcmp(arr2, "quit") == 0){
        //     break;
        // }
        int pipe1[2];
        int pipe2[2];
        int login_pipe[2];
        int sockets[2];
        socketpair(AF_UNIX, SOCK_STREAM, 0, sockets);

        pid_t p;

        if (pipe(pipe1) == -1)
        {
            printf("Pipe Failed");
            return 6;
        }
        if (pipe(pipe2) == -1)
        {
            printf("Pipe Failed");
            return 6;
        }

        p = fork();
        if (p > 0) // parinte
        {   
            close(sockets[CHILD]);
            write(sockets[PARENT], arr2, sizeof(arr2));
            
            if (pipe(login_pipe) == -1)
            {
                printf("Pipe Failed");
                return 6;
            }
            close(login_pipe[write_end]);
            char buffer[1];
            read(login_pipe[read_end], buffer, 1);
            if (buffer[0] == '1')
            {
                ok = true;
            }
            else if (buffer[0] == '0')
            {
                ok = false;
            }
            close(pipe1[read_end]);
            write(pipe1[write_end], arr2, strlen(arr2) - 1);
            close(pipe1[write_end]);
            wait(NULL);
            close(pipe2[write_end]);
            int lungime = read(pipe2[read_end], raspuns, sizeof(raspuns));
            close(pipe2[read_end]);
            raspuns[lungime] = '\0';

            read(sockets[PARENT], raspuns, sizeof(raspuns));

            if (strcmp(raspuns, "utilizatorul a fost logat cu succes") == 0)
            {
                ok = true;
            }
            else if (strcmp(raspuns, "v-ati delogat cu succes") == 0)
            {
                ok = false;
            }
            write(fd_to_client, raspuns, strlen(raspuns) + 1);
            wait(NULL);
        }
        else if (p == 0) // copil
        {
            close(pipe1[write_end]);
            char incercarea[100];
            int lungime = read(pipe1[read_end], incercarea, 100);
            incercarea[lungime] = '\0';
            //printf("%s %d\n", incercarea, strlen(incercarea));
            close(sockets[PARENT]);
            lungime = read(sockets[CHILD], incercarea, 100);
            incercarea[lungime] = '\0';
            //printf("quitu prin socket %s\n", incercarea);
            close(pipe1[read_end]);
            // close(pipe2[read_end]);
            if (strncmp(incercarea, "quit", 4) == 0)
            {
                write(sockets[CHILD], "serverul a fost oprit", 21);
                ok = false;
                return 0;;
            }
            else if (strncmp(incercarea, "login", 5) == 0)
            {
                FILE *usernames = fopen("usernames.txt", "r");
                char user[100];
                strcpy(user, incercarea + 8);
                user[strlen(user) - 1 ] = '\0';
                //printf("%s %d\n", user, strlen(user));
                char user_from_DB[5000];
                fgets(user_from_DB, sizeof(user_from_DB), usernames);
                char *k;
                if (!ok)
                {
                    k = strtok(user_from_DB, ",");
                    while (k != NULL)
                    {   
                        if (strcmp(user, k) == 0)
                        {
                            ok = true;
                            break;
                        }
                        k = strtok(NULL, ",");
                        
                    }
                }
                else
                {
                    close(pipe2[read_end]);
                    write(pipe2[write_end], "utilizatorul a fost deja logat", 30);
                    close(pipe2[write_end]);
                }

                if (!ok)
                {
                    close(pipe2[read_end]);
                    write(pipe2[write_end], "utilizatorul nu a fost recunoscut", 33);
                    close(pipe2[write_end]);
                }
                else
                {
                    close(pipe2[read_end]);
                    write(pipe2[write_end], "utilizatorul a fost logat cu succes", 35);
                    close(pipe2[write_end]);
                }
            }
            else if (strcmp(incercarea, "get-logged-users\n") == 0)
            {   
                
                if (ok)
                {
                    struct utmp *logged_users;
                    setutent();
                    logged_users = getutent();
                    char userInfo[1000];
                    while (logged_users)
                    {
                        char ut_user[32], ut_host[32];

                        if (logged_users->ut_type == USER_PROCESS)
                        {
                            strncpy(ut_user, logged_users->ut_user, 32);
                            strncpy(ut_host, logged_users->ut_host, 32);
                            long secunde = logged_users->ut_tv.tv_sec;
                            snprintf(userInfo, 999, "Nume: %s Host: %s Timp: %25s", ut_user, ut_host, ctime(&secunde));
                        }
                        logged_users = getutent();
                    }
                    close(pipe2[read_end]);
                    write(pipe2[write_end], userInfo, strlen(userInfo));
                    close(pipe2[write_end]);
                }
                else
                {
                    close(pipe2[read_end]);
                    write(pipe2[write_end], "Nu sunteti logat pentru a executa comanda", 41);
                    close(pipe2[write_end]);
                }
            }
            else if (strncmp(incercarea, "logout", 6) == 0 && ok)
            {
                close(pipe2[read_end]);
                write(pipe2[write_end], "v-ati delogat cu succes", 23);
                close(pipe2[write_end]);
                ok = false;
            }
            else if (strncmp(incercarea, "logout", 6) == 0 && !ok)
            {   
                close(pipe2[read_end]);
                write(pipe2[write_end], "nu sunteti logat pentru a va deloga", 35);
                close(pipe2[write_end]);
            }
            else
            {
                close(pipe2[read_end]);
                write(pipe2[write_end], "comanda invalida", 16);
                close(pipe2[write_end]);
            }
            break;
        }
        else
        {
            printf("eroare fork \n");
            return 7;
        }

        
    } while (1);
    close(fd_from_client);
    close(fd_to_client);
    return 0;
}