#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>


#define PORT 42069
#define MESSAGE_SIZE 256
#define CLIENT_MAX 32


static volatile sig_atomic_t gSignalStatus = 1;
static int sock = -1;
pthread_mutex_t lock;


void sighandler(int signal)
{
    gSignalStatus = 0;
}

void *connectionsHandler(void *pFds)
{
    struct pollfd **fds = (struct pollfd **)pFds; 
    printf("Sock: %d\n", sock);
    struct pollfd mainFd[1];
    mainFd[0].fd = sock;
    mainFd[0].events = POLLIN;

    while (gSignalStatus)
    {
        poll(mainFd, 1, -1);
        if (mainFd[0].revents & POLLIN)
        {
            int fd = accept(sock, NULL, NULL);
            if (fd > 0)
            {
    //            pthread_mutex_lock(&lock);
                for (int i = 0; i < CLIENT_MAX; ++i)
                {
                    if ((*fds)[i].fd == -1)
                    {
                        (*fds)[i].fd = fd;
                        printf("Client connected...\n");
                        break;
                    }
                }
    //            pthread_mutex_unlock(&lock);
            }
    
        }
    }
    
    return NULL;
}

int main(void)
{
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (socket <= 0)
    {   
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (const struct sockaddr*)&addr, sizeof(addr)) != 0)
    {
        close(sock);
        return -1;
    }

    if (listen(sock, SOMAXCONN) != 0)
    {
        close(sock);
        return -1;
    }

    signal(SIGINT, sighandler);
    struct pollfd *fds = calloc(CLIENT_MAX, sizeof(struct pollfd));

    for (int i = 0; i < CLIENT_MAX; ++i)
    {
        fds[i].fd = -1;
        fds[i].events = POLLIN;
    }

    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        close(sock);
        free(fds);
        return 1;
    }

    pthread_t tid[1];
    if (pthread_create(&tid[0], NULL, &connectionsHandler, (void *)&fds) != 0)
    {
        free(fds);
        close(sock);
        return -1;
    }

    while (gSignalStatus)
    {
        poll(fds, CLIENT_MAX, -1);
        
  //      pthread_mutex_lock(&lock);
        for (int j = 0; j < CLIENT_MAX; ++j)
        {
            if (fds[j].revents != 0)
            {
                printf("  fd=%d; events: %s%s%s\n", fds[j].fd,
                               (fds[j].revents & POLLIN)  ? "POLLIN "  : "",
                               (fds[j].revents & POLLHUP) ? "POLLHUP " : "",
                               (fds[j].revents & POLLERR) ? "POLLERR " : "");
                if (fds[j].revents & POLLIN)
                {
                    char buf[MESSAGE_SIZE];
                    size_t bytes = read(fds[j].fd, buf, MESSAGE_SIZE);
                    if (bytes > 0)
                    {
                        printf("Got message: %s\n", buf);
                    }
                }
                else
                {
                    close(fds[j].fd);
                    printf("Client disconnected.\n");
                    fds[j].fd = -1;
                    fds[j].revents = 0;
                }
            }
        }
    //    pthread_mutex_unlock(&lock);
    }

    for (int i = 0; i < CLIENT_MAX; ++i)
    {
        printf("In main thread exiting, fds[%d].fd = %d\n", i, fds[i].fd);
        close(fds[i].fd);
    }

    pthread_join(tid[0], NULL);
    close(sock);
    pthread_mutex_destroy(&lock);
    free(fds);

    return 0;
}
