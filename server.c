#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int cycle = 1;
void SigintHandler(int sig)
{
    cycle = 0;
    printf("\nServer out\n");
    exit(0);
}

int err_handler(int func, const char *errstr, int quantity, int *sock)
{
    if(func < 0)
    {
        if(quantity > 0)
        {
            int i;
            for(i = 0; i < quantity; i++)
                close(sock[i]);
        }
        perror(errstr);
        exit(-1);
    }
    return func;
}

int main()
{
    struct sigaction sigint;
    sigint.sa_handler = SigintHandler;
    sigint.sa_flags = 0;
    sigemptyset(&sigint.sa_mask);
    sigaddset(&sigint.sa_mask, SIGINT);
    err_handler(sigaction(SIGINT, &sigint, 0), "sigaction", 0, 0);
    if(signal(SIGCHLD, SIG_IGN) == SIG_ERR)
    {
        perror("signal");
        exit(-1);
    }

    int buf;
    char buf2[16] = "ready\n";
    int sock[2], bytes_read;
    struct sockaddr_in addr, child, parent;
    socklen_t size_p = sizeof(parent), size_c = sizeof(child);
    pid_t child_pid;
    
    sock[0] = err_handler(socket(AF_INET, SOCK_STREAM, 0), "socket", 0, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(0);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    err_handler(bind(sock[0], (struct sockaddr*)&addr, sizeof(addr)), "bind", 0, 0);
    err_handler(getsockname(sock[0], (struct sockaddr*)&parent, &size_p), "getsockname", 0, 0);
    printf("Port: %d\n", ntohs(parent.sin_port));
    listen(sock[0], 6);
    while(cycle)
    {
        sock[1] = err_handler(accept(sock[0], (struct sockaddr*)&child, &size_c), "accept", 1, sock);
        child_pid = err_handler(fork(), "fork", 2, sock);
        if(child_pid == 0)
        {
        	close(sock[0]);
            int i;
            err_handler(send(sock[1], buf2, sizeof(buf2), 0), "send", 1, &sock[1]);
        	for(i = 0; i < 100; i++)
        	{
    	        bytes_read = err_handler(recv(sock[1], &buf, sizeof(int), 0), "recv", 1, &sock[1]);
                printf("%d\n", buf);
            }
            exit(0);
        }
        close(sock[1]);
    }
    close(sock[0]);
    exit(0);
}
