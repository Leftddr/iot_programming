#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>

int main(int argc, char* argv[]){
    int s, client_s;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int addr_len, len;
    char buf[1000];

    s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(s == -1){
        perror("socket");
    }

    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(54321);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(&(server_addr.sin_zero), 0, 0);

    if(bind(s, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        perror("bind");
        return 0;
    }

    if(listen(s, 5) == 1){
        perror("listen");
        return 0;
    }

    addr_len = sizeof(struct sockaddr_in);
    client_s = accept(s, (struct sockaddr *)&client_addr, &addr_len);
    if(client_s == -1){
        perror("accept");
        return 0;
    }

    char op_num;
    read(client_s, &op_num, sizeof(char));
    int *num = (int*)malloc(op_num * sizeof(int));
    read(client_s, num, sizeof(int) * op_num);
    char op;
    read(client_s, &op, sizeof(char));

    int res = 0;

    for(char i = 0 ; i < op_num ; i++){
        num[i] = ntohl(num[i]);
        if(op == '+'){
            res += num[i];
        }
        else if(op == '-'){
            res -= num[i];
        }
        else if(op == '*'){
            res *= num[i];
        }
        else if(op == '/'){
            res /= num[i];
        }
    }

    printf("%d\n", res);

    close(client_s);
    close(s);

    return 0;
}