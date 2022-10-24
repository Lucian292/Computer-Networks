#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
    //crearea fisierului fifo si verificarea daca exista inainte
    if (mkfifo("myfifo1.txt", 0777) == -1) {
        if (errno != EEXIST) {
            printf("Imposibil de creat fisierul fifo1\n");
            return 1;
        }
    }
    //char * myfifo = "myfifo1.txt"; //pointer catre fisierul fifo
    int fd_to_server, fd_from_server;
    char arr1[100], raspunsul[256];
    while (1){
        fd_to_server = open("myfifo1.txt", O_WRONLY);
        if (fd_to_server == -1) {return 2;}
        fgets(arr1, 100, stdin); // citim de la tastatura un cuvant
        write(fd_to_server, arr1, strlen(arr1)+1);
        if (fd_to_server == -1) {return 3;}
        
        fd_from_server = open ("myfifo2.txt", O_RDONLY);
        if (fd_from_server == -1) {return 4;}
        read(fd_from_server, raspunsul, sizeof(char) * 255);
        
        if (strstr(raspunsul, "serverul a fost oprit")){
            printf("Am primit raspunsul: %s\n", raspunsul);
            return 0;
        }
        else {
            printf("Am primit raspunsul: %s\n", raspunsul);
        }
        //read(fd, arr1, sizeof(arr1));
        close (fd_to_server);
        close (fd_from_server);
    }
    return 0;
}