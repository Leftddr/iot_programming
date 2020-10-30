#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include <errno.h>

void zombie_handler(){
    int status;
    int spid;
    spid = wait(&status);
}

int main(int argc, char* argv[]){
    int s, client_s;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int addr_len, len;
    char buf[1000];

    struct sigaction act;
    act.sa_handler = zombie_handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);
    sigaction(SIGCHLD, &act, 0);

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
    while(1){
        client_s = accept(s, (struct sockaddr *)&client_addr, &addr_len);
        if(client_s < 0){
            switch(errno){
                case EINTR:
                    continue;
                default:
                    perror("accept");
            }
        }
        if(client_s == -1){
            perror("accept");
            return 0;
        }

        if(fork() == 0){
            //부모꺼 닫기
            close(s);
            while(1){
                len = read(client_s, buf, sizeof(buf));
                if(len == -1){
                    perror("read");
                    return 0;
                }
                else if(len == 0){
                    printf("Disconnected\n");
                    break;
                }

                //null은 아스키 코드 0번이다.
                buf[len] = '\0';
                printf("recv : %s\n", buf);

                len = write(client_s, buf, len);
                if(len == -1){
                    perror("write");
                    return 0;
                }
            }
            //자신의 소켓 닫기
            close(client_s);
            exit(1);
        }
        else{
            //자식 소켓 닫기
            close(client_s);
        }
    }
    close(s);

    return 0;
}