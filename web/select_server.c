#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <errno.h>

#define BUF_SIZE 100

char escape[] = "exit\n";

int main(int argc, char* argv[]){
    int sockfd, new_fd;
    struct sockaddr_in my_addr, their_addr;
    unsigned int sin_size, len_inet;
    int pid, size;
    char buf[BUF_SIZE];
    fd_set readfds, readtemp;
    int max_fd = 0;
    int result;

    if(argc < 2){
        fprintf(stderr, "Usage : %s <PORT>\n", argv[0]);
        return 0;
    }

    sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd == -1){
        perror("socket");
    }

    my_addr.sin_family = PF_INET;
    my_addr.sin_port = htons(atoi(argv[1]));
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(&(my_addr.sin_zero), 0, 0);

    if(bind(sockfd, (struct sockaddr*)&my_addr, sizeof(my_addr)) == -1){
        perror("bind");
        return 0;
    }

    if(listen(sockfd, 5) == 1){
        perror("listen");
        return 0;
    }

    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    max_fd = sockfd + 1;

    sin_size = sizeof(struct sockaddr_in);
    printf("Waiting...\n");

    while(1){
        readtemp = readfds;
        result = select(max_fd, &readtemp, NULL, NULL, NULL);
        if(result == -1){
            perror("select");
            return 0;
        }
        for(int i = 0 ; i < max_fd ; i++){
            if(FD_ISSET(i, &readtemp)){
                if(i == sockfd){
                    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
                    if(new_fd < 0){
                        perror("accept");
                    }
                    printf("Connected\n");
                    FD_SET(new_fd, &readfds);
                    if(max_fd < new_fd)
                        max_fd = new_fd;
                }
                else{
                    size = recv(i, buf, BUF_SIZE, 0);
                    if(size > 0){
                        buf[size] = '\0';
                        printf("Recv : %s\n", buf);
                        send(i, buf, size, 0);
                    }
                    else if(size == 0){
                        FD_CLR(i, &readfds);
                        close(i);
                        printf("disconnected\n");
                    }
                    else{
                        perror("recv");
                        return 0;
                    }
                }
            }
        }      
    }
    close(sockfd);

    return 0;
}