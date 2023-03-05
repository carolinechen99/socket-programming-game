#include "potato.hpp"
#include "socketutils.hpp"

using namespace std;

// keep listening to the three channels, and receive potato
int Player::handlePotato(){

    Potato potato;
    int num_players = -1;
    num_players = getNumPlayers();
    int master_sockfd = -1;
    master_sockfd = getMasterSockfd();
    int prev_sockfd = -1;
    prev_sockfd = getPrevSockfd();
    int next_sockfd = -1;
    next_sockfd = getNextSockfd();

    while (true){

    int status = -1;
    // create a set of file descriptors
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(master_sockfd, &readfds);
    FD_SET(prev_sockfd, &readfds);
    FD_SET(next_sockfd, &readfds);

    // find the max fd
    int max_fd = max(master_sockfd, prev_sockfd);
    max_fd = max(max_fd, next_sockfd);

    // select() to check if there is any msg from ringmaster or neighbors
    status = select(max_fd + 1, &readfds, NULL, NULL, NULL);
    if (status == -1) {
        cerr << "Error: cannot select" << endl;
        return -1;
    }

    //debug
    cout << "select() returns" << endl;

    // if there is a msg from ringmaster, receive the potato
    if (FD_ISSET(master_sockfd, &readfds)) {
        status = recv(master_sockfd, &potato, sizeof(potato), MSG_WAITALL);
        if (status == -1) {
            cerr << "Error: cannot receive potato from ringmaster" << endl;
            return -1;
        }
        //debug

        cout << "potato from ringmaster: " << potato.nhops << endl;

        // if received potato with nhops = 0, return and shut down the game
        if (potato.nhops == 0) {

            //debug
            cout << "Received potato with nhops = 0 end game" << endl;
            return 0;
        }
    }

    // if there is a msg from previous player, receive the potato
    else if (FD_ISSET(prev_sockfd, &readfds)) {
        status = recv(prev_sockfd, &potato, sizeof(potato), MSG_WAITALL);
        if (status == -1) {
            cerr << "Error: cannot receive potato from previous player" << endl;
            return -1;
        }
    }

    // if there is a msg from next player, receive the potato
    else if (FD_ISSET(next_sockfd, &readfds)) {
        status = recv(next_sockfd, &potato, sizeof(potato), MSG_WAITALL);
        if (status == -1) {
            cerr << "Error: cannot receive potato from next player" << endl;
            return -1;
        }
    }


        potato.addTrace(player_id);
        potato.nhops--;
        cout << "add trace potato hops: " << potato.getHops() << endl;
        cout << "potato fix hops: " << potato.getFixHop() << endl;
        potato.printTrace();
    // if nhops > 0, add the player id to the path, pass to random neighbor
    if (potato.nhops > 0) {

        // pass to random neighbor
        // seed the random number generator
        srand(time(NULL));
        int random_neighbor = rand() % 2;
        if (random_neighbor == 0) {
            status = send(next_sockfd, &potato, sizeof(potato), 0);
            if (status == -1) {
                cerr << "Error: cannot send potato to next player" << endl;
                return -1;
            }

            // print "Sending potato to <player_id>"
            cout << "Sending potato to " << (player_id + 1) % num_players << endl;
        }
        else {
            status = send(prev_sockfd, &potato, sizeof(potato), 0);
            if (status == -1) {
                cerr << "Error: cannot send potato to previous player" << endl;
                return -1;
            }

            // print "Sending potato to <player_id>"
            cout << "Sending potato to " << (player_id + num_players - 1) % num_players << endl;
        }
    }

    else if (potato.nhops == 0) {
        // print "I'm it"
        cout << "I'm it" << endl;
        // add player id to the path
        potato.addTrace(player_id);
        // send potato back to ringmaster
        status = send(master_sockfd, &potato, sizeof(potato), 0);
        if (status == -1) {
            cerr << "Error: cannot send potato to ringmaster" << endl;
            return -1;
        }

        //debug
        cout << "Sent potato to ringmaster" << endl;
        cout << "potato hops: " << potato.getHops() << endl;
        cout << "potato fix hops: " << potato.getFixHop() << endl;
        potato.printTrace();
        cout << "it potato trace printed" << endl;
        cout << "after print potato hops: " << potato.getHops() << endl;
        cout << "after print potato fix hops: " << potato.getFixHop() << endl;
    }

    else {
        cerr << "Error: nhops < 0" << endl;
        return -1;
    }
    }
    return 0;
}

// connec to neighbors
int Player::connectToNeighbors(Server &player_server){
    // connect to next player
    int status = connectNextPlayer();
    if (status == -1) {
        cerr << "Error: cannot connect to next player" << endl;
        return -1;
    }

    // accept connection from previous player
    status = acceptPrevPlayer(player_server);
    if (status == -1) {
        cerr << "Error: cannot accept connection from previous player" << endl;
        return -1;
    }

    // send msg to ringmaster to indicate that player is ready
    const char *msg = "ready";
    int master_sockfd = getMasterSockfd();
    status = send(master_sockfd, msg, sizeof(msg), 0);
    if (status == -1) {
        cerr << "Error: cannot send ready msg to ringmaster" << endl;
        return -1;
    }

    return 0;
}



// accept connection from previous player
int Player::acceptPrevPlayer(Server &player_server){
    // accept connection from previous player
    struct sockaddr_storage prev_addr;
    socklen_t prev_addr_len = sizeof(prev_addr);
    int master_sockfd = getMasterSockfd();
    int prev_sockfd = -1;
    prev_sockfd = accept(player_server.socket_fd, (struct sockaddr *)&prev_addr, &prev_addr_len);
    if (prev_sockfd == -1) {
        cerr << "Error: cannot accept connection on socket" << endl;
        return -1;
    }
    // set prev_sockfd
    setPrevSockfd(prev_sockfd);
    return 0;
}

// connect to next player
int Player::connectNextPlayer(){
    // receive next player's hostname and port number
    char next_hostname [256];
    char next_port [100];
    memset(next_hostname, 0, sizeof(next_hostname));
    memset(next_port, 0, sizeof(next_port));
    int master_sockfd = getMasterSockfd();
    int status = recv(master_sockfd, next_hostname, sizeof(next_hostname), MSG_WAITALL);
    if (status == -1) {
        cerr << "Error: cannot receive next_hostname from ringmaster" << endl;
        return -1;
    }

    status = recv(master_sockfd, next_port, sizeof(next_port), MSG_WAITALL);
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
        cerr << "Error: cannot get address info for next host" << endl;
        cerr << "  (" << next_hostname << "," << next_port << ")" << endl;
        return -1;
    } //if

    // loop through all the results and connect to the first we can
    struct addrinfo *p;
    int next_sockfd = -1;
    
    for(p = host_info_list; p != NULL; p = p->ai_next) {
        next_sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (next_sockfd == -1) {
            continue;
        } //if

        status = connect(next_sockfd, p->ai_addr, p->ai_addrlen);
        if (status == -1) {
            close(next_sockfd);
            continue;
        } //if
        break;
    } //for

    if (p == NULL) {
        cerr << "Error: failed to connect socket" << endl;
        return -1;
    } //if

    // set next_sockfd
    setNextSockfd(next_sockfd);

    freeaddrinfo(host_info_list);
    return 0;

}

//connect to the ringmaster
int Player::connectToRingmaster(const char *machine_name, const char *master_port, Server &player_server){
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    int socket_fd = -1;
    int status = -1;

    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(machine_name, master_port, &host_info, &host_info_list);
    if (status != 0) {
        cerr << "Error: cannot get address info for master host" << endl;
        cerr << "  (" << machine_name << "," << master_port << ")" << endl;
        return -1;
    } //if

    // loop through all the results and connect to the first we can
    struct addrinfo *p;
    for(p = host_info_list; p != NULL; p = p->ai_next) {
        socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socket_fd == -1) {
            continue;
        } //if

        status = connect(socket_fd, p->ai_addr, p->ai_addrlen);
        if (status == -1) {
            close(socket_fd);
            continue;
        } //if
        break;
    } //for

    if (p == NULL) {
        cerr << "Error: player failed to connect ring master socket" << endl;
        return -1;
    } //if

    // receive() id and numPlayers from ringmaster
    size_t id = 0;
    size_t numPlayers;
    status = recv(socket_fd, &id, sizeof(id), 0);
    if (status == -1) {
        cerr << "Error: cannot receive id from ringmaster" << endl;
        return -1;
    }
    status = recv(socket_fd, &numPlayers, sizeof(numPlayers), 0);
    if (status == -1) {
        cerr << "Error: cannot receive numPlayers from ringmaster" << endl;
        return -1;
    }

    //print `Connected as player x out of n total players`
    cout << "Connected as player " << id << " out of " << numPlayers << " total players" << endl;


    // set numPlayers
    setNumPlayers(numPlayers);

    // set id 
    setPlayerId(id);

    // set master socket
    setMasterSockfd(socket_fd);

    // get host name
    char hostname[256];
    memset(hostname, 0, sizeof(hostname));
    if (gethostname(hostname, sizeof(hostname)) == -1) {
        cerr << "Error: cannot get hostname" << endl;
        return -1;
    }
    setHostname(hostname);

    // get port number
    int port = player_server.port;
    string port_str = to_string(port);
    char port_num[100];
    memset(port_num, 0, sizeof(port_num));
    strcpy(port_num, port_str.c_str());
    setPortNum(port_num);


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

    
    freeaddrinfo(host_info_list);
    return 0;

}


// check the content of command line arguments
int checkPlayerArg(const char *machine_name, char * port_num) {
    // check port number
    int num = atoi(port_num);
    if (num < 1024 || num > 65535) {
        cerr << "Error: port number should be between 1024 and 65535" << endl;
        return -1;
    }
    return 0;
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
    if (player_server->createSocket((char*)"0") == -1) {
        cerr << "Error: cannot create player socket" << endl;
        return -1;
    }

    // connect to the ringmaster
    if (player->connectToRingmaster(machine_name, port_num, *player_server) == -1) {
        cerr << "Error: cannot connect to ringmaster" << endl;
        return -1;
    }

    // connect to neighbor players
    if (player->connectToNeighbors(*player_server) == -1) {
        cerr << "Error: cannot connect to neighbor player" << endl;
        return -1;
    }

    // initialize a potato

    if (player->handlePotato() == -1) {
        cerr << "Error: cannot handle potato" << endl;
        return -1;
    }

    // close socket
    close(player->master_sockfd);
    close(player->next_sockfd);
    close(player->prev_sockfd);


    // free memory
    delete player;
    delete player_server;
    return 0;
}