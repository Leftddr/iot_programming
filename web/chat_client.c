#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#define BUF_SIZE 100

typedef struct header{
    uint16_t len;
    char type;
}HEADER;

void *thread_send(void *arg)
{
    int size;
    char buf[BUF_SIZE];
    int server_s = *(int *)arg;
    HEADER header;

    while((size = read(0, buf, BUF_SIZE)) > 0){
        ////////////////////////////////////////////////////
        buf[size - 1] = '\0';
        if(strcmp(buf, "/q") == 0){
            printf("exit\n");
            header.len = htons(0);
            header.type = 'q';
            //header만 보낸다.
            send(server_s, (char*)&header, sizeof(HEADER), 0);

            shutdown(server_s, SHUT_WR);
            break;
        }
        else{
            header.len = htons(strlen(buf));
            header.type = 'm';
            char *send_msg = (char*)malloc(sizeof(char) * header.len);
            strcpy(send_msg, buf);

            //header, msg 순서로 메세지를 보낸다.
            send(server_s, (char*)&header, sizeof(HEADER), 0);
            send(server_s, send_msg, strlen(send_msg), 0);
            free(send_msg);
        }

        ////////////////////////////////////////////////////////
    }

    return NULL;
}

int main(int argc, char* argv[]){
    
    int sockfd;
    struct sockaddr_in my_addr;
    unsigned int sin_size;
    char buf[BUF_SIZE];

    if(argc < 3){
        fprintf(stderr, "Usage : %s <IP> <PORT> <NAME>\n", argv[0]);
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

    printf("Connected. (/q)\n");
    
    ///////////////////////////////////////////////////////////////
    HEADER header;
    header.len = htons(strlen(argv[3]));
    header.type = 'c';
    char *send_msg = (char *)malloc(sizeof(char) * header.len);
    strcpy(send_msg, argv[3]);

    //처음 connect할 때 메세지 순서는 header, msg 순서대로 보낸다.
    send(sockfd, (char*)&header, sizeof(HEADER), 0);
    send(sockfd, send_msg, strlen(send_msg), 0);
    /////////////////////////////////////////////////////////////////

    pthread_t t_id;
    pthread_create(&t_id, NULL, thread_send, (void*)&sockfd);
    pthread_detach(t_id);

    while(1){
        //recv에서 block 되어 있다.
        //여기서는 그냥 echo server에서 받은 것을 그대로 띄워준다.
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
        printf("%s\n", buf);
    }
    
    close(sockfd);
    return 0;
}