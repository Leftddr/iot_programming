#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char* argv[]){
    struct hostent *host;
    struct sockaddr_in addr;

    inet_aton("172.217.163.238", &addr.sin_addr);
    host = gethostbyaddr((char*)&addr.sin_addr, sizeof(addr.sin_addr), PF_INET);

    if(host == NULL){
        herror("gethostbyname");
        return 0;
    }

    printf("name : %s\n", host->h_name);
    for(int i = 0 ; host->h_aliases[i] != NULL ; i++){
        printf("alias %d : %s\n", i, host->h_aliases[i]);
    }

    printf("type : %d\n", host->h_addrtype);
    printf("length : %d\n", host->h_length);

    for(int i = 0; host->h_addr_list[i] != NULL ; i++){
        printf("ip %d: %s\n", i, inet_ntoa(*(struct in_addr*)host->h_addr_list[i]));
    }
    return 0;
}