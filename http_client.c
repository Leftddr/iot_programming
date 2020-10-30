#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]){
    if(argc < 2){
        printf("Input Address\n");
        return 0;
    }

    char request[1000] = "GET / HTTP/1.1\r\nHost: ";
    char *Connection = "Connection: close\r\n\r\n";
    char msg[1500];
    char address[100];
    strcpy(address, argv[1]);
    strcat(argv[1], "\r\n");
    strcat(request, argv[1]);
    strcat(request, Connection);

    int s;
    struct sockaddr_in server_addr;

    printf("%s\n", request);

    s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(80);
    inet_aton(address, &server_addr.sin_addr);
    memset(&(server_addr.sin_zero), 0, 0);

    if(connect(s, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        perror("connect");
        return 0;
    }

    int len = write(s, request, strlen(request));
    if(len == -1){
        perror("write");
        return 0;
    }

    while(1){
        int read_len = read(s, msg, sizeof(msg));
        if(read_len == -1){
            perror("read");
            close(s);
            return 0;
        }
        else if(read_len == 0){
            break;
        }
        
        printf("%s", msg);
    }
    printf("\n");
    close(s);
    return 0;
}