#include "socketutils.hpp"

using namespace std;

    int Server::createSocket(const char *port){
        int status;
        int socket_fd = -1;
        struct addrinfo host_info;
        struct addrinfo *host_info_list;
        const char *hostname = NULL;
        int portNum;

        memset(&host_info, 0, sizeof(host_info));

        host_info.ai_family   = AF_UNSPEC;
        host_info.ai_socktype = SOCK_STREAM;
        host_info.ai_flags    = AI_PASSIVE;

        status = getaddrinfo(hostname, port, &host_info, &host_info_list);
        if (status != 0) {
            cerr << "Error: cannot get address info for host --server" << endl;
            return -1;
        } //if

        // loop through all the results and bind to the first we can
        struct addrinfo *p;
        int yes = 1;
        for(p = host_info_list; p != NULL; p = p->ai_next) {
            socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (socket_fd == -1) {
                continue;
            } //if

            
            if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
                cerr << "Error: cannot set socket option" << endl;
                return -1;
            } //if

            if (bind(socket_fd, p->ai_addr, p->ai_addrlen) == -1) {
                close(socket_fd);
                continue;
            } //if
            break;
        } //for

        
        portNum = getPortNum(socket_fd);
        setPort(portNum);

        if (p == NULL) {
            cerr << "Error: failed to bind socket" << endl;
            return -1;
        } //if

        status = listen(socket_fd, SOMAXCONN);
        if (status == -1) {
            cerr << "Error: cannot listen on socket" << endl; 
            cerr << "  (" << hostname << "," << port << ")" << endl;
            return -1;
        } //if


        // set server's socket_fd
        setSocketFd(socket_fd);  

        freeaddrinfo(host_info_list);
        return 0;

    }

    int Server::getPortNum(int socket_fd){
        struct sockaddr_in sin;
        socklen_t len = sizeof(sin);
        if (getsockname(socket_fd, (struct sockaddr *)&sin, &len) == -1){
            cerr << "Error: cannot get socket name" << endl;
            return -1;
        }
        int port = ntohs(sin.sin_port);
        //print port for debugging
        return port;
    }

