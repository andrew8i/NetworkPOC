#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>

#include <iostream>

extern "C"
{
#include "unistd.h"
}

#include "test.h"


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QTcpServer *mServer = new QTcpServer(&a);

    mServer->listen(QHostAddress::AnyIPv4, PORT);
    if (mServer->isListening())
    {
        QObject::connect(mServer, &QTcpServer::newConnection, [mServer]()
            {
                QTcpSocket *sock = mServer->nextPendingConnection();
                QObject::connect(sock, &QAbstractSocket::readyRead, [sock]()
                    {
                        char buf[MAX_MSG_SIZE];
                        sock->read(buf, MAX_MSG_SIZE);
                        std::cout << "Received: " << buf << '\n';
                    
                        if (!strcmp(buf, "exit"))
                        {
                            memset(buf, 0, MAX_MSG_SIZE);
                            strcpy(buf, "Thanks! Closing my end...");
                            sock->write(buf, MAX_MSG_SIZE);
                            sock->waitForBytesWritten(-1);
                            delete sock;
                        }
                        else
                        {
                            memset(buf, 0, MAX_MSG_SIZE);
                            strcpy(buf, "Thanks!");
                            sock->write(buf, MAX_MSG_SIZE);
                            sock->waitForBytesWritten(-1);
                        }
                        
                    }
                );
            }
        );
    
    }

    return a.exec();
}
