#include <arpa/inet.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "test.h"

static volatile sig_atomic_t running = 1;

static void handler(int signum)
{
    printf("Shutting down...\n");
    running = 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Missing IP address\n");
        return -1;
    }

    int sockfd;
    struct pollfd fds[1];
    struct sockaddr_in servaddr;
    
    const struct sigaction act  = {.sa_handler = (void*)&handler};
    sigaction(SIGINT, &act, NULL);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd <= 0)
    {
        perror("Socket creation failed...");
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(argv[1]);
    servaddr.sin_port = htons(PORT);
reconnect:
    if (connect(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) != 0)
    {
        static int count = 1;
        fprintf(stderr, "Initial connection with the server failed... (attempts: %d)\n", count);
        sleep(1);
        count++;
        if (count == 6)
        {
            fprintf(stderr, "Shutting down.\n");
            close(sockfd);
            return -1;
        }
        else
        {
            goto reconnect;
        }
    }
    else
    {
        fprintf(stdout, "Connected to the server... (IP: %s)\n", argv[1]);
    }

    fds[0].fd = sockfd;
    fds[0].events = POLLIN;

    char buf[MAX_MSG_SIZE];
    while(running)
    {
        memset(buf, 0, MAX_MSG_SIZE);

        printf("Send to server: ");
        fgets(buf, MAX_MSG_SIZE, stdin);
        if ((strlen(buf) > 0) && (buf[strlen(buf) - 1] == '\n'))
        {
            buf[strlen(buf) - 1] = '\0';
        }

        write(fds[0].fd, buf, MAX_MSG_SIZE);

        poll(fds, 1, -1);
        if (fds[0].revents != 0)
        {
            if (fds[0].revents == POLLIN)
            {
                memset(buf, 0, MAX_MSG_SIZE);
                read(fds[0].fd, (void *)buf, MAX_MSG_SIZE);
                printf("Received: %s\n\n", buf);
            }
            else if (fds[0].revents == (POLLIN | POLLHUP | POLLERR))
            {
                fprintf(stdout, "Other end closed connection\n");
                running = 0;
            }
        }
    }

    close(sockfd);

    return 0;

}
