#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int sock[2];
void SigintHandler(int sig)
{
    printf("\nServer out\n");
    close(sock[0]);
    exit(0);
}

int err_handler(int func, const char *errstr, int quantity, int first)
{
    if(func < 0)
    {
        if(quantity > 0)
        {
            int i;
            for(i = first; i < quantity; i++)
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
    int bytes_read;
    struct sockaddr_in addr, child, parent;
    socklen_t size_p = sizeof(parent), size_c = sizeof(child);
    pid_t child_pid;
    
    sock[0] = err_handler(socket(AF_INET, SOCK_STREAM, 0), "socket", 0, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(0);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    err_handler(bind(sock[0], (struct sockaddr*)&addr, sizeof(addr)), "bind", 1, 0);
    err_handler(getsockname(sock[0], (struct sockaddr*)&parent, &size_p), "getsockname", 1, 0);
    printf("Port: %d\n", ntohs(parent.sin_port));
    listen(sock[0], 6);
    while(1)
    {
        sock[1] = err_handler(accept(sock[0], (struct sockaddr*)&child, &size_c), "accept", 1, 0);
        child_pid = err_handler(fork(), "fork", 2, 0);
        if(child_pid == 0)
        {
        	close(sock[0]);
            int i, n;
            err_handler(send(sock[1], buf2, sizeof(buf2), 0), "send", 1, 1);
            bytes_read = err_handler(recv(sock[1], &n, sizeof(int), 0), "recv", 1, 1);
        	for(i = 0; i < n; i++)
        	{
    	        bytes_read = err_handler(recv(sock[1], &buf, sizeof(int), 0), "recv", 1, 1);
                printf("%d\n", buf);
            }
            exit(0);
        }
        close(sock[1]);
    }
}
