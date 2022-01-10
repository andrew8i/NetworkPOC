#include <iostream>
#include <QCoreApplication> 
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

#define PORT 42069
#define MESSAGE_SIZE 256
#define CLIENT_MAX 32

/*
 * The listener should probably be the object responsible with holding the QTcpServer object
 * and looking after all the clients that are connected. Possibly storing this in an array
 * of clients.
 *
 * When the client DCs then kill that connection and remove it from the array perhaps.
 * This will also be responsible with updating the database using BoG. So we'll need 
 * to link in BoG.
 *
 *
 * Don't forget to add in the installer using libssh.
 */


class A : public QObject
{
    Q_OBJECT

public:
    explicit A(QObject *parent = nullptr) : QObject(parent)
    {
        server = new QTcpServer(this);
        server->listen(QHostAddress::Any, PORT);
        if (server->isListening())
        {
           this->mIsListening = true;
           connect(server, &QTcpServer::newConnection, this, &A::sendMessage);
        }
    }
    virtual ~A()
    {
       delete server;
    }

    bool isListening() { return this->mIsListening; }

public slots:
    void sendMessage() 
    {
        QTcpSocket *sock = this->server->nextPendingConnection();
        std::cout << "Client connected with IP: " << sock->peerAddress().toString().toStdString() << '\n';
        connect(sock, &QAbstractSocket::disconnected, sock, &QObject::deleteLater);

        sock->write(QByteArray("Hello, World!\n"));

        sock->disconnectFromHost();
    }

private:
    QTcpServer *server = nullptr;
    bool mIsListening = false;

};

#include "qtserver.moc"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    A a(&app);
    if (!a.isListening())
    {
        std::cout << "Server not listening\n"; 
        return -1;
    }
    else
    {
        std::cout << "Server is listening for connections...\n";
    }


    return app.exec();
}
