#include <netinet/ip.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/signalfd.h>
#include <sys/types.h>
#include <unistd.h>

#include "test.h"


static volatile sig_atomic_t running = 1;
static int sock = -1;

static void intHandler(int signum)
{
    running = 0;
}

//void *acceptor(void *vargs)
//{
//    struct pollfd **fds = (struct pollfd **)(vargs);
//    printf("Started acceptor thread with pollfd address: %p\n", &(*fds));
//    int fd = accept(sock, NULL, NULL);
//    if (fd > 0)
//    {
//        printf("New connection! With fd: %d\n", fd);
//        for (int i = 0; i < MAX_CLIENT_CONS; ++i)
//        {
//            if ((*fds)[i].fd == -1)
//            {
//                printf("Found free space at fds[%d]\n", i);
//                (*fds)[i].fd = fd;
//                printf("set fds[%d]->fd to %d\n", i, fd);
//                clientCount++;
//                raise(SIGUSR1);                    
//                break;
//            }
//        }
//    }
//
//    return NULL;
//}


int main(int argc, char *argv[])
{
    printf("Starting server. Listening on port: %d...\n", PORT);

    const struct sigaction act = {.sa_handler = intHandler};
    sigaction(SIGINT, &act, NULL);
    int rc = 1;
//    sigset_t mask;
//    int signal_fd;
//    struct signalfd_siginfo signal_info_fd;

  //  pthread_t tid;

//    sigemptyset(&mask);
//    sigaddset(&mask, SIGUSR1);
//    sigprocmask(SIG_BLOCK, &mask, NULL);
//
//    signal_fd = signalfd(-1, &mask, 0);

    struct pollfd *fds = calloc(sizeof(struct pollfd), MAX_CLIENT_CONS + 1);
    printf("Started main thread with pollfd address: %p\n", &fds);
    for (int i = 0; i < MAX_CLIENT_CONS; ++i)
    {
        fds[i].fd = -1;
        fds[i].events = POLLIN;
    }

//    fds[MAX_CLIENT_CONS].fd = signal_fd;
//    fds[MAX_CLIENT_CONS].events = POLLIN;

    sock = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    printf("Created new socket: %d\n", sock);

    fds[MAX_CLIENT_CONS].fd = sock;
    fds[MAX_CLIENT_CONS].events = POLLIN;

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    rc = bind(sock, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    printf("%s\n", (!rc ? "Bound address to a socket..." : "Unable to bind socket..."));

    rc = listen(sock, MAX_CLIENT_CONS);
    printf("%s\n", (!rc ? "Listening for new connections..." : "Unable to listen for new connections..."));

//    pthread_create(&tid, NULL, acceptor, (void *)&fds);

    while (running)
    {
        poll(fds, MAX_CLIENT_CONS + 1, -1);

        if (fds[MAX_CLIENT_CONS].revents != 0
                && fds[MAX_CLIENT_CONS].revents & POLLIN)
        {
            int fd = accept(sock, NULL, NULL);
            if (fd > 0)
            {
                printf("New connection! With fd: %d\n", fd);
                for (int i = 0; i < MAX_CLIENT_CONS; ++i)
                {
                    if (fds[i].fd == -1)
                    {
                        printf("Found free space at fds[%d]\n", i);
                        fds[i].fd = fd;
                        printf("set fds[%d]->fd to %d\n", i, fd);
                        break;
                    }
                }
            }
        }
        for (int i = 0; i < MAX_CLIENT_CONS; ++i)
        {
            if (fds[i].revents != 0)
            {
                if (fds[i].revents & POLLIN)
                {
                    char buf[MAX_MSG_SIZE];
                    read(fds[i].fd, (void *)buf, MAX_MSG_SIZE);
                    printf("Recieved: %s (from fd: %d)\n", buf, fds[i].fd);

                    memset(buf, 0, MAX_MSG_SIZE);
                    strcpy(buf, "Thanks!");
                    write(fds[i].fd, (void *)buf, MAX_MSG_SIZE);
                }
                else if (fds[i].revents & (POLLHUP | POLLERR))
                {
                    printf("Other end closed...\n");
                    close(fds[i].fd);
                    fds[i].fd = -1;
                }
            }
        }
    }

    for (int i = 0; i < MAX_MSG_SIZE + 1; ++i)
    {
        close(fds[i].fd);
    }

    return 0;
}
