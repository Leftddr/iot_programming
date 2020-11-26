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
#define MAX 100

char escape[] = "exit\n";

//client 소켓 정보와 fd를 저장할 변수
struct sockaddr_in clients[MAX] = {0, };
int clients_fd[MAX] = {0, };
char *client_name[MAX] = {NULL, };
int clients_num = 0;

typedef struct header{
    uint16_t len;
    char type;
}HEADER;

int recv_header(int server_s, HEADER *header){
    //int size = recv(server_s, (char*)header, sizeof(HEADER), 0);
    int size = recv(server_s, &(header->len), 2, 0);
    size += recv(server_s, &(header->type), 1, 0);
    header->len = ntohs(header->len);
    return size;
}

int recv_msg(int server_s, char *msg, int len){
    int size = recv(server_s, msg, len, 0);
    return size;
}

void store_client_name(int sockfd, char* name, int len){
    for(int i = 0 ; i < MAX ; i++){
        if(clients_fd[i] == sockfd){
            client_name[i] = (char*)malloc(sizeof(char) * len);
            strcpy(client_name[i], name);
            printf("%s\n", client_name[i]);
            break;
        }
    }
    return;
}

char* get_client_name(int sockfd){
    for(int i = 0 ; i < MAX ; i++){
        if(clients_fd[i] == sockfd){
            return client_name[i];
        }
    }
    return NULL;
}

void store_client(struct sockaddr_in client, int sockfd){
    //최대 클라이언트 수를 넘으면 저장하지 않고 나간다.
    if(clients_num >= MAX) return;
    for(int i = 0 ; i < MAX ; i++){
        if(clients_fd[i] == 0){
            clients_fd[i] = sockfd;
            clients[i] = client;
            clients_num++;
            break;
        }
    }
    return;
}

void erase_client(int sockfd){
    for(int i = 0 ; i < MAX ; i++){
        if(clients_fd[i] == sockfd){
            clients_fd[i] = 0;
            //client_name을 프리시켜 준다.
            free(client_name[i]);
            clients_num--;
            break;
        }
    }
    return;
}

void send_to_all(char *msg){
    for(int i = 0 ; i < MAX ; i++){
        if(clients_fd[i] == 0) continue;
        HEADER header;
        header.len = strlen(msg);
        header.len = htons(header.len);
        header.type = 'm';
        send(clients_fd[i], (char*)&header.len, 2, 0);
        send(clients_fd[i], (char*)&header.type, 1, 0);
        send(clients_fd[i], msg, strlen(msg), 0);
    }
}

void *thread_main(void *arg){
    int server_s = *(int *)arg;
    free(arg);

    HEADER header;

    while(1){
        int size1 = recv_header(server_s, &header);
        int size2 = 0;
        char *msg;
        if(header.type != 'q'){
            printf("소켓 유지\n");
            msg = (char *)malloc(sizeof(char) * header.len);
            size2 = recv_msg(server_s, msg, header.len);
            if(header.type == 'c'){
                store_client_name(server_s, msg, header.len);
            }
        }

        if(size1 == 0 || size2 == 0){
            printf("Disconnected\n");
            //여기서 client를 지운다.
            erase_client(server_s);
            break;
        }
        else if(size1 == -1 || size2 == 0){
            perror("read");
            //여기서 client를 지운다.
            erase_client(server_s);
            break;
        }

        printf("Recv : %s\n", msg);
        if(header.type == 'm'){
            char *name = get_client_name(server_s);
            //client 이름과 같이 보낸다.
            char *send_msg = (char*)malloc(sizeof(char) * (strlen(msg) + strlen(name) + 2));
            strcat(send_msg, name);
            strcat(send_msg, ": ");
            strcat(send_msg, msg);
            /////////////////////////
            send_to_all(send_msg);
            free(send_msg);
        }
        free(msg);
        fflush(stdout);
        //send(server_s, buf, size, 0);
    }

    //소켓 종료!!
    close(server_s);
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

        //여기서 변수에 저장한다.
        store_client(client_addr, clientfd);

        arg = (int *)malloc(sizeof(int));
        *(int *)arg = clientfd;

        pthread_create(&t_id, NULL, thread_main, arg);
        pthread_detach(t_id);
    }

    close(sockfd);
    return 0;
}