#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 100

char escape[] = "exit\n";

void *thread_send(void *arg)
{
    while(1){
        int size;
        char buf[BUF_SIZE];
        int server_s = *(int *)arg;

        printf("Input : ");
        fflush(stdout);
        while((size = read(0, buf, BUF_SIZE)) > 0){
            buf[size] = '\0';

            if(strncmp(buf, escape, 4) == 0){
                shutdown(server_s, SHUT_WR);
                break;
            }

            send(server_s, buf, size, 0);
            printf("HOST : %s", buf);
            fflush(stdout);
            printf("Input : ");
            fflush(stdout);
        }
    }

    return NULL;
}

int main(int argc, char* argv[]){
    int sockfd;
    struct sockaddr_in my_addr;
    unsigned int sin_size;
    char buf[BUF_SIZE];

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

    if(connect(sockfd, (struct sockaddr *)&my_addr, sin_size) < 0){
        perror("connect");
        return 0;
    }

    printf("Connected. (Enter \\ exit \\ quit to quit)\n");

    pthread_t t_id;
    pthread_create(&t_id, NULL, thread_send, (void*)&sockfd);
    pthread_detach(t_id);

    while(1){
        int size = recv(sockfd, buf, BUF_SIZE, 0);
        if(size == 0){
            printf("Disconnected\n ");
            break;
        }
        if(size == -1){
            perror("Error\n");
            return 0;
        }
        buf[size] = '\0';
        printf("Server : %s", buf);
    }

    close(sockfd);
    return 0;
}