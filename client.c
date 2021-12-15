#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 42069
#define MESSAGE_SIZE 256

volatile sig_atomic_t gSignalStatus = 1;

void sighandler(int signal)
{
    gSignalStatus = 0;
}

int main (int argc, char *argv[])
{
    if (argc != 2)
    {
        return -1;
    }

    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock <= 0)
    {
        return -1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sock, (const struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        close(sock);
        return -1;
    }

    signal(SIGINT, sighandler);

    char buff[MESSAGE_SIZE];
    while (gSignalStatus)
    {
        memset(buff, 0, sizeof(buff));
        printf("Enter the string : ");
        scanf("%s", buff);
        write(sock, buff, sizeof(buff));

//        memset(buff, 0, sizeof(buff));
//        read(sock, buff, sizeof(buff));
//        printf("From Server : %s", buff);

        if ((strncmp(buff, "exit", 4)) == 0)
        {
            memset(buff, 0, sizeof(buff));
            printf("Client Exit...\n");
            strcpy(buff, "Good-bye");
            write(sock, buff, sizeof(buff));

            close(sock);
            gSignalStatus = 0;
        }
    }


    return 0;
}
