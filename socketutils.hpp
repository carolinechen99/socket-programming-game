#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define BACKLOG 1020

class Server{
    public:
    ~Server(){}

    // create socket and bind to port
    // Reference: tcp_example/server.cpp
    int createSocket(char *port, bool isPlayer);
};