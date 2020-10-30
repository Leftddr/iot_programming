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
    int addr_len;
    char msg[1000] = "hello world";

    s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(s == -1){
        perror("socket");
        return 0;
    }

    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(54321);
    inet_aton("127.0.0.1", &server_addr.sin_addr);
    memset(&(server_addr.sin_zero), 0, 0);

    if(connect(s, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        perror("connect");
        return 0;
    }

    write(s, msg, strlen(msg) + 1);
    close(s);

    return 0;
}