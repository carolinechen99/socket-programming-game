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


    ~Server(){}

    // create socket and bind to port
    // Reference: tcp_example/server.cpp
    int createSocket(char *port, bool isPlayer, Server &server);

    // get port number
    int getPortNum(int socket_fd);

    // get hostname
    char *getHostname();

};

#endif