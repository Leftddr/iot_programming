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
#include <fcntl.h>
#include <malloc.h>

#define MAX_LEN 1000000

void zombie_handler(){
    int status;
    int spid;
    spid = wait(&status);
}

void make_error(char *send_msg){
    FILE *fs;
    fs = fopen("./error.html", "r");
    strcat(send_msg, "HTTP/1.1 404 Not Found\r\n");
    strcat(send_msg, "Content-Type: text/html\r\n");
    strcat(send_msg, "Content-Length: ");

    char data[MAX_LEN] = {0, };

    while (!feof(fs)){
        char str[100];
        fgets(str, 100, fs);
        if(strlen(data) + strlen(str) >= MAX_LEN) break;
        strcat(data, str);
    }

    char data_len[10];
    sprintf(data_len, "%lu", strlen(data));
    strcat(send_msg, data_len);
    strcat(send_msg, "\r\n\r\n");
    strcat(send_msg, data);
    fclose(fs);
}

int main(int argc, char* argv[]){
    if(argc < 2){
        printf("Usage : ./web_server [PORT]");
        exit(0);
    }

    int s, client_s;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    int addr_len;

    /*
    zombie process의 처리를 위해 signal 처리 함수를 만든다.
    */
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
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(&(server_addr.sin_zero), 0, 8);

    if(bind(s, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1){
        perror("bind");
        return 0;
    }

    if(listen(s, 5) == -1){
        perror("listen");
        return 0;
    }

    addr_len = sizeof(struct sockaddr_in);
    //multi process 구현
    while(1){
        client_s = accept(s, (struct sockaddr*)&client_addr, &addr_len);
        //client_s의 값을 보고 accept로 돌아올지 오류가 났는지 판단한다.
        if(client_s < 0){
            switch(errno){
                case EINTR:
                    continue;
                default:
                    perror("accept");
            }
        }

        if(fork() == 0){
            close(s);
            while(1){
                char recv_msg[MAX_LEN] = {0, }, send_msg[MAX_LEN] = {0, };
                int len = read(client_s, recv_msg, sizeof(recv_msg));
                printf("%s\n", recv_msg);
                if(len == -1){
                    perror("read");
                    return 0;
                }
                else if(len == 0){
                    printf("Disconnected\n");
                    break;
                }
                char *filename = strtok(recv_msg, " ");
                filename = strtok(NULL, " ");
                if(!strcmp(filename, "/")){
                    FILE *fs;
                    fs = fopen("./index.html", "r");
                    if(fs == NULL){
                        memset(send_msg, 0, MAX_LEN);
                        make_error(send_msg);
                    }
                    else{
                        memset(send_msg, 0, MAX_LEN);
                        strcat(send_msg, "HTTP/1.1 200 OK\r\n");
                        strcat(send_msg, "Content-Type: text/html\r\n");
                        strcat(send_msg, "Content-Length: ");

                        char data[MAX_LEN] = {0, };
                        while (!feof(fs)){
                            char str[100];
                            fgets(str, 100, fs);
                            if(strlen(data) + strlen(str) >= MAX_LEN) break;
                            strcat(data, str);
                        }

                        char data_len[10];
                        sprintf(data_len, "%lu", strlen(data));
                        strcat(send_msg, data_len);
                        strcat(send_msg, "\r\n\r\n");
                        strcat(send_msg, data);
                        fclose(fs);
                    }
                    len = write(client_s, send_msg, strlen(send_msg));
                    printf("\n%s\n", send_msg);
                    if(len == -1){
                        perror("write");
                        return 0;
                    }
                }
                else{
                    int to_send_len = 0;
                    //type구분.
                    char filetype[5];
                    int i_ = 0;
                    int filename_len = strlen(filename);
                    for(int i = filename_len - 4 ; i < filename_len ; i++, i_++){
                        filetype[i_] = filename[i];
                    }
                    char filename_[15] = {0, };
                    filename_[0] = '.'; filename_[1] = '/';
                    i_ = 2;
                    for(int i = 1 ; i < filename_len ; i++, i_++){
                        filename_[i_] = filename[i];
                    }

                    if(!strcmp(filetype, ".png")){
                        memset(send_msg, 0, MAX_LEN);
                        strcat(send_msg, "HTTP/1.1 200 OK\r\n");
                        strcat(send_msg, "Content-Type: image/png\r\n");
                        strcat(send_msg, "Content-Length: ");

                        FILE* fs;
                        fs = fopen(filename_, "rb");
                        if(fs == NULL) make_error(send_msg);
                        else{
                            fseek(fs, 0, SEEK_END);
                            long long img_size = ftell(fs);
                            rewind(fs);

                            char *data = (char*)malloc(sizeof(char) * img_size);
                            if(data == NULL){
                                perror("Memory Error");
                                exit(1);
                            }
                            //binary file의 길이는 strlen으로 구할 수 없다. 
                            long long result = fread(data, 1, img_size, fs);
                            if(result != img_size){
                                perror("Reading error");
                                exit(1);
                            }

                            char data_len[10];
                            sprintf(data_len, "%lld", img_size);
                            strcat(send_msg, data_len);
                            strcat(send_msg, "\r\n\r\n");
                            //data 복사
                            to_send_len = 0;
                            to_send_len += strlen(send_msg);
                            to_send_len += img_size;
                            
                            int send_msg_index = strlen(send_msg);
                            for(int i = 0 ; i < img_size ; i++, send_msg_index++){
                                send_msg[send_msg_index] = data[i];
                            }

                            fclose(fs);
                            printf("final end\n");
                        }
                    }
                    else if(!strcmp(filetype, ".jpg")){
                        memset(send_msg, 0, MAX_LEN);
                        strcat(send_msg, "HTTP/1.1 200 OK\r\n");
                        strcat(send_msg, "Content-Type: image/jpg\r\n");
                        strcat(send_msg, "Content-Length: ");

                        FILE* fs;
                        fs = fopen(filename_, "rb");
                        if(fs == NULL) make_error(send_msg);
                        else{
                            fseek(fs, 0, SEEK_END);
                            long long img_size = ftell(fs);
                            fseek(fs, 0, SEEK_SET);

                            char *data = (char*)malloc(sizeof(char) * img_size);
                            if(data == NULL){
                                perror("Memory Error");
                                exit(1);
                            }
            
                            long long result = fread(data, 1, img_size, fs);
                            if(result != img_size){
                                perror("Reading error");
                                exit(1);
                            }

                            char data_len[10];
                            sprintf(data_len, "%lld", img_size);
                            strcat(send_msg, data_len);
                            strcat(send_msg, "\r\n\r\n");
                            //data 복사
                            to_send_len = 0;
                            to_send_len += strlen(send_msg);
                            to_send_len += img_size;
                            
                            int send_msg_index = strlen(send_msg);
                            for(int i = 0 ; i < img_size ; i++, send_msg_index++){
                                send_msg[send_msg_index] = data[i];
                            }

                            fclose(fs);
                            printf("final end\n");
                        }
                    }
                    else if(!strcmp(filetype, "favicon.ico")){
                        printf("favicon error\n");
                        continue;
                    }
                    else{
                        printf("type error\n");
                        memset(send_msg, 0, MAX_LEN);
                        make_error(send_msg);
                        break;
                    }

                    len = write(client_s, send_msg, to_send_len);
                    printf("%s\n", send_msg);
                    if(len == -1){
                        perror("write");
                        return 0;
                    }
                }
            }
            close(client_s);
            exit(1);
        }
        else{
            close(client_s);
        }
    }
    
    close(s);
    return 0;
}