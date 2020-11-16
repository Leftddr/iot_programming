#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include <malloc.h>

#define BUF_SIZE 100

char escape[] = "exit\n";

void *thread_main(void *arg){
    int server_s = *(int *)arg;
    free(arg);

    char buf[BUF_SIZE];

    while(1){
        int size = recv(server_s, buf, BUF_SIZE, 0);
        if(size == 0){
            printf("Disconnected\n");
            break;
        }
        else if(size == -1){
            perror("read");
            return NULL;
        }

        buf[size] = '\0';
        printf("Recv : %s", buf);
        fflush(stdout);
        send(server_s, buf, size, 0);
    }

}

int main(int argc, char* argv[]){
    int sockfd, clientfd;
    struct sockaddr_in my_addr, client_addr;
    unsigned int sin_size;
    char buf[BUF_SIZE];
    int *arg;

    if(argc < 3){
        fprintf(stderr, "Usage : %s <IP> <PORT>\n", argv[0]);
        return 0;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(atoi(argv[2]));
    my_addr.sin_addr.s_addr = inet_addr(argv[1]);
    memset(&(my_addr.sin_zero), 0, 8);

    sin_size = sizeof(struct sockaddr_in);

    if(bind(sockfd, (struct sockaddr *)&my_addr, sin_size) < 0){
        perror("bind");
        return 0;
    }

    if(listen(sockfd, 5) < 0){
        perror("listen");
        return 0;
    }

    pthread_t t_id;

    while(1){
        clientfd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
        if(clientfd < 0){
            perror("accept");
            return 0;
        }

        printf("Connected : %s %d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);

        arg = malloc(sizeof(int));
        *(int *)arg = clientfd;

        pthread_create(&t_id, NULL, thread_main, arg);
        pthread_detach(t_id);
    }

    close(sockfd);
    return 0;
}