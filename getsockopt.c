#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

int main(int argc, char* argv[]){
    int tcp_sock, udp_sock;
    int opt, optlen = 4;

    tcp_sock = socket(PF_INET, SOCK_STREAM, 0);
    udp_sock = socket(PF_INET, SOCK_DGRAM, 0);

    printf("TCP : %d\n", SOCK_STREAM);
    getsockopt(tcp_sock, SOL_SOCKET, SO_TYPE, (void *)&opt, &optlen);
    printf("Socket : %d\n", opt);
    getsockopt(tcp_sock, SOL_SOCKET, SO_SNDBUF, (void *)&opt, &optlen);
    printf("Send Buffer Size : %d\n", opt);
    getsockopt(tcp_sock, SOL_SOCKET, SO_RCVBUF, (void *)&opt, &optlen);
    printf("Recv buffer Size : %d\n", opt);

    opt = 1000000;
    setsockopt(tcp_sock, SOL_SOCKET, SO_SNDBUF, (void*)&opt, optlen);
    setsockopt(tcp_sock, SOL_SOCKET, SO_RCVBUF, (void*)&opt, optlen);

    getsockopt(tcp_sock, SOL_SOCKET, SO_SNDBUF, (void *)&opt, &optlen);
    printf("Send Buffer Size : %d\n", opt);
    getsockopt(tcp_sock, SOL_SOCKET, SO_RCVBUF, (void *)&opt, &optlen);
    printf("Recv buffer Size : %d\n", opt);

    printf("UDP : %d\n", SOCK_DGRAM);
    getsockopt(udp_sock, SOL_SOCKET, SO_RCVBUF, (void *)&opt, &optlen);
    printf("Socket : %d\n", opt);
    getsockopt(udp_sock, SOL_SOCKET, SO_SNDBUF, (void *)&opt, &optlen);
    printf("Send buffer Size : %d\n", opt);
    getsockopt(udp_sock, SOL_SOCKET, SO_RCVBUF, (void *)&opt, &optlen);
    printf("Recv buffer Size : %d\n", opt);
}