#include "potato.hpp"
#include "socketutils.hpp"

using namespace std;

// shut dowm the game
int Ringmaster::shutDownGame(){
    Potato potato;
    int status = -1;
    size_t num_players = getNumPlayers();
    vector <int> player_fds = getPlayerFds();
    // send potato with nhops = 0 to all players to shut down the game
    for (size_t i = 0; i < num_players; i++) {
        status = send(player_fds[i], &potato, sizeof(potato), 0);
        if (status == -1) {
            cerr << "Error: cannot send potato 0 to player" << endl;
            return -1;
        }
    }
    return 0;
}

// receive potato from the last player
int Ringmaster::receivePotato(Potato &potato){
    // using select() to check if there is any player sending potato
    size_t num_players = getNumPlayers();
    int status = -1;
    vector <int> player_fds = getPlayerFds();

    fd_set readfds;
    FD_ZERO(&readfds);

    for (size_t i = 0; i < num_players; i++) {
        FD_SET(player_fds[i], &readfds);
    }

    int max_fd = 0;
    for (size_t i = 0; i < num_players; i++) {
    FD_SET(player_fds[i], &readfds);
    if (player_fds[i] > max_fd) {
        max_fd = player_fds[i];
    }
}

    status = select(max_fd + 1, &readfds, NULL, NULL, NULL);
    if (status == -1) {
        cerr << "Error: cannot select" << endl;
        return -1;
    }


    // If there is a player sending potato, receive the potato
    for (size_t i = 0; i < num_players; i++) {
        if (FD_ISSET(player_fds[i], &readfds)) {
            status = recv(player_fds[i], &potato, sizeof(Potato), MSG_WAITALL);
            if (status == -1) {
                cerr << "Error: cannot receive potato from player" << endl;
                return -1;
            }

            // Shut down the game if potato has 0 hops
            if (potato.getHops() == 0) {
                shutDownGame();
                return 0;
            }
        }
    }

    return 0;
}


int Ringmaster::launchPotato(size_t num_hops){
    Potato potato(num_hops);
    size_t num_players = getNumPlayers();
    int status = -1;
    vector<int> player_fds = getPlayerFds();
    vector<bool> player_ready(num_players, false);

    if (num_hops == 0){
        return 0;
    }


    srand(time(NULL));
    size_t random_player;
    random_player = rand() % num_players;
    status = send(player_fds[random_player], &potato, sizeof(Potato), 0);
    if (status == -1) {
        cerr << "Error: cannot send potato to player" << endl;
        return -1;
    }

    cout << "Ready to start the game, sending potato to player " << random_player << endl;
    return 0;
}



// create ring process
int Ringmaster::createRing(Server &master_server){
    size_t id = 0;
    size_t num_players = getNumPlayers();
    vector <string> player_hostnames = getPlayerHostnames();
    vector <string> player_port_nums = getPlayerPortNums();
    char next_hostname_cstr[256];
    
    char next_port_cstr[100];
    
    while (id < num_players) {
        // send next player's hostname and port number to current player
        string next_hostname = player_hostnames[(id + 1) % num_players];
        string next_port = player_port_nums[(id + 1) % num_players];
        memset(next_hostname_cstr, 0, sizeof(next_hostname_cstr));
        strcpy(next_hostname_cstr, next_hostname.c_str());
        memset(next_port_cstr, 0, sizeof(next_port_cstr));
        strcpy(next_port_cstr, next_port.c_str());
        int status = send(player_fds[id], next_hostname_cstr, sizeof(next_hostname_cstr), 0);
        if (status == -1) {
            cerr << "Error: cannot send next_hostname to player" << endl;
            return -1;
        }

        status = send(player_fds[id], next_port_cstr, sizeof(next_port_cstr), 0);
        if (status == -1) {
            cerr << "Error: cannot send next_port to player" << endl;
            return -1;
        }

        id++;
    }

    return 0;

}

// connect to the players
int Ringmaster::connectToPlayers(Server &master_server){
    size_t count = 0;
    size_t id = 0;
    size_t numPlayers = getNumPlayers();

    char hostname[256];
    char port[100];
    while (count < num_players) {
        // accept connection from players
        struct sockaddr_storage player_addr;
        socklen_t player_addr_len = sizeof(player_addr);
        int player_fd = accept(master_server.socket_fd, (struct sockaddr *)&player_addr, &player_addr_len);
        if (player_fd == -1) {
            cerr << "Error: cannot accept connection on socket" << endl;
            return -1;
        }
        // add player_fd to player_fds
        addPlayerFd(player_fd);

        // send id and numPlayers to players
        size_t id = count;
        int status = send(player_fd, &id, sizeof(id), 0);
        if (status == -1) {
            cerr << "Error: cannot send id to player" << endl;
            return -1;
        }
        status = send(player_fd, &numPlayers, sizeof(numPlayers), 0);
        if (status == -1) {
            cerr << "Error: cannot send numPlayers to player" << endl;
            return -1;
        }

        // receive() hostname and port number from players
        memset(hostname, 0, sizeof(hostname));
        memset(port, 0, sizeof(port));

        status = recv(player_fd, hostname, sizeof(hostname), MSG_WAITALL);
        if (status == -1) {
            cerr << "Error: cannot receive hostname from player" << endl;
            return -1;
        }
        status = recv(player_fd, port, sizeof(port), MSG_WAITALL);
        if (status == -1) {
            cerr << "Error: cannot receive port from player" << endl;
            return -1;
        }

        // add hostname and port number to the list of player_hostnames and player_ports
        string hostname_cstr(hostname);
        string port_cstr(port);

        addPlayerHostname(hostname_cstr);
        addPlayerPortNum(port_cstr);

        cout << "Player " << count << " is ready to play" << endl;

        count++;
    }

    return 0;
}



// check content of command line arguments
int checkRMArg(const char *port_num, size_t num_player, size_t num_hops){
    // check port number
    int num = atoi(port_num);
    if (num < 1024 || num > 65535) {
        cerr << "Error: port number should be between 1024 and 65535" << endl;
        return -1;
    }

    // check number of players: greater than 1
    if (num_player < 2) {
        cerr << "Error: number of players should be greater than 1" << endl;
        return -1;
    }

    // check number of hops: greater than or equal to 0 and less than or equal to 512
    if (num_hops < 0 || num_hops > 512) {
        cerr << "Error: number of hops should be greater than or equal to 0 and less than or equal to 512" << endl;
        return -1;
    }
    return 0;

}



// server program is invoked as: ringmaster <port_num> <num_players> <num_hops>
// take argv[1] as port_num, argv[2] as num_players, argv[3] as num_hops
int main(int argc, char *argv[]) {
    // check argument number
    if (argc != 4) {
        cerr << "Usage: ringmaster <port_num> <num_players> <num_hops>" << endl;
        exit(1);
    }

    // assign them to ringmaster
    const char *port_num = argv[1];
    size_t num_players = atoi(argv[2]);
    size_t num_hops = atoi(argv[3]);
    // check the content of command line arguments
    checkRMArg(port_num, num_players, num_hops);

                // print out the ringmaster info
            std::cout << "Potato Rringmaster" << std::endl;
            std::cout << "Players = " << num_players << std::endl;
            std::cout << "Hops = " << num_hops << std::endl;
    // initialize the ringmaster
    Ringmaster *ringmaster = new Ringmaster(port_num, num_players, num_hops);
    // set up the ringmaster server
    Server * rm_server = new Server();
    rm_server->createSocket(port_num);

    // assign port number to the ringmaster
    ringmaster->setPortNum(port_num);
    // assign number of players to the ringmaster
    ringmaster->setNumPlayers(num_players);
    // assign number of hops to the ringmaster
    ringmaster->num_hops = num_hops;

    // initialize the potato
    Potato potato(num_hops);

    // connect to the players
    ringmaster->connectToPlayers(*rm_server);

    //create ring process
    ringmaster->createRing(*rm_server);


    // launch the potato to the first random player
    ringmaster->launchPotato(num_hops);

        //check if nhops is 0
    if (num_hops == 0) {
        ringmaster->shutDownGame();
        // close all the sockets
        for (size_t i = 0; i < ringmaster->num_players; i++) {
            close(ringmaster->player_fds[i]);
        }
        // close(rm_server->socket_fd); // debug

        delete ringmaster;
        delete rm_server;

        return 0;
    }

    // receive the potato from the last player
    ringmaster->receivePotato(potato);

    // close all the sockets
    for (size_t i = 0; i < ringmaster->num_players; i++) {
        close(ringmaster->player_fds[i]);
    }
    // close(rm_server->socket_fd); // debug

    potato.printTrace();

    delete ringmaster;
    delete rm_server;

    return 0;
}