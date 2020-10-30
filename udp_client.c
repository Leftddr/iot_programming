#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]){
    int s;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int addr_len, len, read_len;
    char msg[1000];

    s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(s == -1){
        perror("socket");
        return 0;
    }

    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(54321);
    inet_aton("127.0.0.1", &server_addr.sin_addr);
    memset(&(server_addr.sin_zero), 0, 0);

    while(1){
        printf("input : ");
        fgets(msg, sizeof(msg), stdin);
        if(strcmp(msg, "q\n") == 0 || strcmp(msg, "Q\n") == 0){
            close(s);
            return 0;
        }
        len = sendto(s, msg, strlen(msg), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if(len == -1){
            perror("sendto");
            return 0;
        }
        
        read_len = recvfrom(s, msg, sizeof(msg), 0, (struct sockaddr*)&client_addr, &addr_len);
        if(read_len == -1){
            perror("recvfrom");
            return 0;
        }
        else if(read_len == 0){
            printf("Disconnected\n");
            break;
        }
        msg[read_len] = 0;
        printf("%s", msg);
    }

    close(s);
    return 0;
}