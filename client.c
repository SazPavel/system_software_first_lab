#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char **argv)
{
    int sock, bytes_read, j, i;
    char buf[16];
    struct sockaddr_in addr;
    if (argc < 2)
    {
        printf("Usage:\n./client <IP> <port> <i>\n");
        exit(0);
    }
    i = atoi(argv[3]);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        perror("sock");
        exit(-1);
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    if(connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        exit(-1);
    }
    bytes_read = recv(sock, buf, 16, 0);
    printf("%s\n", buf);
    for(j = 0; j < 100; j++)
    {
        printf("%d\n", i);
        send(sock, &i, sizeof(i), 0);
        sleep(i);
    }
    close(sock);
    exit(0);
}
