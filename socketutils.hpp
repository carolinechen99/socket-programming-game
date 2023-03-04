#ifndef SOCKETUTILS_H
#define SOCKETUTILS_H

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define BACKLOG 1020

class Server{
    public:
    int socket_fd;
    int port;

    Server(): socket_fd(-1), port(-1) {}

    ~Server(){
        close(socket_fd);
    }

    // create socket and bind to port
    // Reference: tcp_example/server.cpp
    int createSocket(const char *port);

    // get port number
    int getPortNum(int socket_fd);

    // set socket_fd
    void setSocketFd(int socket_fd){
        this->socket_fd = socket_fd;
    }

    // set port
    void setPort(int port){
        this->port = port;
    }

    // get socket_fd
    int getSocketFd(){
        return socket_fd;
    }

    // get port
    int getPort(){
        return port;
    }

};

#endif