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

int main(int argc, char *argv[])
{
    printf("Starting server. Listening on port: %d...\n", PORT);

    const struct sigaction act = {.sa_handler = (void *)intHandler};
    sigaction(SIGINT, &act, NULL);
    int rc = 1;

//    struct pollfd *fds = calloc(sizeof(struct pollfd), MAX_CLIENT_CONS + 1);
    struct pollfd *fds = calloc(MAX_CLIENT_CONS + 1, sizeof(struct pollfd));

    printf("Started main thread with pollfd address: %p\n", &fds);
    for (int i = 0; i < MAX_CLIENT_CONS; ++i)
    {
        fds[i].fd = -1;
        fds[i].events = POLLIN;
    }

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
                printf("fds[%d].revents: 0x%04x\n", i, fds[i].revents);
                if (fds[i].revents == POLLIN)
                {
                    char buf[MAX_MSG_SIZE];
                    read(fds[i].fd, (void *)buf, MAX_MSG_SIZE);
                    printf("Recieved: %s (from fd: %d)\n", buf, fds[i].fd);

                    memset(buf, 0, MAX_MSG_SIZE);
                    strcpy(buf, "Thanks!");
                    write(fds[i].fd, (void *)buf, MAX_MSG_SIZE);
                }
                else if (fds[i].revents == (POLLIN | POLLHUP | POLLERR))
                {
                    printf("Other end closed...\n");
                    close(fds[i].fd);
                    fds[i].fd = -1;
                }
            }
        }
    }

    for (int i = 0; i < MAX_CLIENT_CONS + 1; ++i)
    {
        if (fds[i].fd != -1)
        {
            printf("fds[%d].fd:\tclose(%d);\n", i, fds[i].fd);
            close(fds[i].fd);
        }
    }

    return 0;
}
