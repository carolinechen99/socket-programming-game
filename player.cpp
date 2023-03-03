#include "potato.hpp"
#include "socketutils.hpp"

using namespace std;

// connec to neighbors
int Player::connectToNeighbors(Player &player){
    // connect to next player
    int status = connectNextPlayer(player);
    if (status == -1) {
        cerr << "Error: cannot connect to next player" << endl;
        return -1;
    }

    // accept connection from previous player
    status = acceptPrevPlayer(player);
    if (status == -1) {
        cerr << "Error: cannot accept connection from previous player" << endl;
        return -1;
    }

    // send msg to ringmaster to indicate that player is ready
    const char *msg = "ready";
    status = send(player.master_sockfd, msg, sizeof(msg), 0);
    if (status == -1) {
        cerr << "Error: cannot send ready msg to ringmaster" << endl;
        return -1;
    }

    return 0;
}



// accept connection from previous player
int Player::acceptPrevPlayer(Player &player){
    // accept connection from previous player
    struct sockaddr_storage prev_addr;
    socklen_t prev_addr_len = sizeof(prev_addr);
    player.prev_sockfd = accept(player.master_sockfd, (struct sockaddr *)&prev_addr, &prev_addr_len);
    if (player.prev_sockfd == -1) {
        cerr << "Error: cannot accept connection on socket" << endl;
        return -1;
    }
    return 0;
}

// connect to next player
int Player::connectNextPlayer(Player &player){
    // receive next player's hostname and port number
    char *next_hostname = new char[1024];
    char *next_port = new char[1024];
    int status = recv(player.master_sockfd, next_hostname, sizeof(next_hostname), MSG_WAITALL);
    if (status == -1) {
        cerr << "Error: cannot receive next_hostname from ringmaster" << endl;
        return -1;
    }
    status = recv(player.master_sockfd, next_port, sizeof(next_port), MSG_WAITALL);
    if (status == -1) {
        cerr << "Error: cannot receive next_port from ringmaster" << endl;
        return -1;
    }

    // connect to next player as a client
    struct addrinfo host_info;
    struct addrinfo *host_info_list;

    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(next_hostname, next_port, &host_info, &host_info_list);
    if (status != 0) {
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << next_hostname << "," << next_port << ")" << endl;
        return -1;
    } //if

    // loop through all the results and connect to the first we can
    struct addrinfo *p;
    for(p = host_info_list; p != NULL; p = p->ai_next) {
        player.next_sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (player.next_sockfd == -1) {
            continue;
        } //if

        status = connect(player.next_sockfd, p->ai_addr, p->ai_addrlen);
        if (status == -1) {
            close(player.next_sockfd);
            continue;
        } //if
        break;
    } //for

    if (p == NULL) {
        cerr << "Error: failed to connect socket" << endl;
        return -1;
    } //if

    freeaddrinfo(host_info_list);
    return 0;

}

//connect to the ringmaster
int connectToRingmaster(char *machine_name, char *master_port, Player &player, Server &player_server){
    struct addrinfo host_info;
    struct addrinfo *host_info_list;

    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    int status = getaddrinfo(machine_name, master_port, &host_info, &host_info_list);
    if (status != 0) {
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << machine_name << "," << master_port << ")" << endl;
        return -1;
    } //if

    int socket_fd = socket(host_info_list->ai_family, 
                host_info_list->ai_socktype, 
                host_info_list->ai_protocol);
    if (socket_fd == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << machine_name << "," << master_port << ")" << endl;
        return -1;
    } //if

    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        cerr << "Error: cannot connect to socket" << endl;
        cerr << "  (" << machine_name << "," << master_port << ")" << endl;
        return -1;
    } //if

    // receive() id and numPlayers from ringmaster
    size_t id;
    int numPlayers;
    status = recv(socket_fd, &id, sizeof(int), 0);
    if (status == -1) {
        cerr << "Error: cannot receive id from ringmaster" << endl;
        return -1;
    }
    status = recv(socket_fd, &numPlayers, sizeof(int), 0);
    if (status == -1) {
        cerr << "Error: cannot receive numPlayers from ringmaster" << endl;
        return -1;
    }

    //print `Connected as player x out of n total players`
    cout << "Connected as player " << id << " out of " << numPlayers << " total players" << endl;

    freeaddrinfo(host_info_list);


    // set id 
    player.player_id = id;

    // set master socket
    player.master_sockfd = socket_fd;

    // get host name
    char hostname[1024];
    hostname[1023] = '\0';
    if (gethostname(hostname, 1023) == -1) {
        cerr << "Error: cannot get host name" << endl;
        return -1;
    }
    player.hostname = hostname;

    // get port number
    int port = player_server.port;
    string port_str = to_string(port);
    char *port_num = new char[port_str.length() + 1];
    strcpy(port_num, port_str.c_str());
    player.port_num = port_num;
    delete [] port_num;


    // send hostname and port number to ringmaster
    status = send(socket_fd, hostname, sizeof(hostname), 0);
    if (status == -1) {
        cerr << "Error: cannot send hostname to ringmaster" << endl;
        return -1;
    }
    status = send(socket_fd, port_num, sizeof(port_num), 0);
    if (status == -1) {
        cerr << "Error: cannot send port number to ringmaster" << endl;
        return -1;
    }
    return 0;

}


// check the content of command line arguments
void checkPlayerArg(const char *machine_name, char * port_num) {
    // check port number
    int num = atoi(port_num);
    if (num < 1024 || num > 65535) {
        cerr << "Error: port number should be between 1024 and 65535" << endl;
        exit(1);
    }
}

/* The player program is invoked as:
player <machine_name> <port_num>
(example: ./player vcm-xxxx.vm.duke.edu 1234)*/

int main(int argc, char *argv[]) {
    // check argument number
    if (argc != 3) {
        cerr << "Usage: player <machine_name> <port_num>" << endl;
        exit(1);
    }

    // assign them to player
    char *machine_name = argv[1];
    char *port_num = argv[2];

    // check the content of command line arguments
    checkPlayerArg(machine_name, port_num);

    // initialize the player
    Player *player = new Player();

    // set up player socket
    Server *player_server = new Server();
    player_server->createSocket(NULL, true, *player_server);



}