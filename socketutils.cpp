#include "socketutils.hpp"

using namespace std;

    int Server::createSocket(char *port, bool isPlayer){
        int status;
        int socket_fd;
        struct addrinfo host_info;
        struct addrinfo *host_info_list;
        const char *hostname = NULL;

        memset(&host_info, 0, sizeof(host_info));

        host_info.ai_family   = AF_UNSPEC;
        host_info.ai_socktype = SOCK_STREAM;
        host_info.ai_flags    = AI_PASSIVE;

        status = getaddrinfo(hostname, port, &host_info, &host_info_list);
        if (status != 0) {
            cerr << "Error: cannot get address info for host" << endl;
            cerr << "  (" << hostname << "," << port << ")" << endl;
            return -1;
        } //if

        // if is player, change port to 0 to get a random port
        if (isPlayer){
            ((struct sockaddr_in *)host_info_list->ai_addr)->sin_port = 0;
        }

        socket_fd = socket(host_info_list->ai_family, 
                    host_info_list->ai_socktype, 
                    host_info_list->ai_protocol);
        if (socket_fd == -1) {
            cerr << "Error: cannot create socket" << endl;
            cerr << "  (" << hostname << "," << port << ")" << endl;
            return -1;
        } //if

        int yes = 1;
        status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
        if (status == -1) {
            cerr << "Error: cannot bind socket" << endl;
            cerr << "  (" << hostname << "," << port << ")" << endl;
            return -1;
        } //if

        status = listen(socket_fd, 100);
        if (status == -1) {
            cerr << "Error: cannot listen on socket" << endl; 
            cerr << "  (" << hostname << "," << port << ")" << endl;
            return -1;
        } //if

        freeaddrinfo(host_info_list);
        return 0;

    }