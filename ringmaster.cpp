#include "potato.hpp"
#include "socketutils.hpp"

using namespace std;

// lanch the potato to the first random player
int Ringmaster::launchPotato(Potato potato){
    // receive msg from player to indicate that player is ready
    // upon receiving the msg, print 'Player x is ready to play'
    // since connections are established, 
    // use select() to check if there is any msg from player
    // if the message is 'ready', print 'Player x is ready to play'

    // select() to check if there is any msg from player
    fd_set readfds;
    FD_ZERO(&readfds);
    for (size_t i = 0; i < num_players; i++) {
        FD_SET(player_fds[i], &readfds);
    }
    int max_fd = player_fds[num_players - 1];
    int status = select(max_fd + 1, &readfds, NULL, NULL, NULL);
    if (status == -1) {
        cerr << "Error: cannot select" << endl;
        return -1;
    }

    // if the message is 'ready', print 'Player x is ready to play'
    // note that the print out is not based on the order of player_fds
    // but based on which player sends the msg first
    // after receiving from all players, break the loop, and send potato to the random player
    for (size_t i = 0; i < num_players; i++) {
        if (FD_ISSET(player_fds[i], &readfds)) {
            char *msg = new char[1024];
            status = recv(player_fds[i], msg, sizeof(msg), MSG_WAITALL);
            if (status == -1) {
                cerr << "Error: cannot receive msg from player" << endl;
                return -1;
            }
            if (strcmp(msg, "ready") == 0) {
                cout << "Player " << i << " is ready to play" << endl;
            }
        }
    }

    // send potato to the random player
    // first, send the number of hops to the random player
    // seed the random number generator
    srand(time(NULL));
    int random_player = rand() % num_players;
    status = send(player_fds[random_player], &potato, sizeof(potato), 0);
    if (status == -1) {
        cerr << "Error: cannot send potato to player" << endl;
        return -1;
    }
    // print "Ready to start the game, sending potato to player x"
    cout << "Ready to start the game, sending potato to player " << random_player << endl;
    return 0;
}


// create ring process
int Ringmaster::createRing(Ringmaster &ringmaster, Server &master_server){
    size_t id = 0;
    while (id < ringmaster.num_players) {
        // send next player's hostname and port number to current player
        char *next_hostname = ringmaster.player_hostnames[(id + 1) % ringmaster.num_players];
        char *next_port = ringmaster.player_port_nums[(id + 1) % ringmaster.num_players];
        int status = send(ringmaster.player_fds[id], next_hostname, sizeof(next_hostname), 0);
        if (status == -1) {
            cerr << "Error: cannot send next_hostname to player" << endl;
            return -1;
        }
        status = send(ringmaster.player_fds[id], next_port, sizeof(next_port), 0);
        if (status == -1) {
            cerr << "Error: cannot send next_port to player" << endl;
            return -1;
        }

    }
    return 0;

}

// connect to the players
int Ringmaster::connectToPlayers(Ringmaster &ringmaster, Server &master_server){
    size_t count = 0;
    while (count < ringmaster.num_players) {
        // accept connection from players
        struct sockaddr_storage player_addr;
        socklen_t player_addr_len = sizeof(player_addr);
        int player_fd = accept(master_server.socket_fd, (struct sockaddr *)&player_addr, &player_addr_len);
        if (player_fd == -1) {
            cerr << "Error: cannot accept connection on socket" << endl;
            return -1;
        }

        // add player_fd to the list of player_fds
        ringmaster.player_fds.push_back(player_fd);

        // send id and numPlayers to players
        size_t id = count;
        int numPlayers = ringmaster.num_players;
        int status = send(player_fd, &id, sizeof(int), 0);
        if (status == -1) {
            cerr << "Error: cannot send id to player" << endl;
            return -1;
        }
        status = send(player_fd, &numPlayers, sizeof(int), 0);
        if (status == -1) {
            cerr << "Error: cannot send numPlayers to player" << endl;
            return -1;
        }

        // receive() hostname and port number from players
        char hostname[1024];
        char port[1024];
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
        ringmaster.player_hostnames.push_back(hostname);
        ringmaster.player_port_nums.push_back(port);

        count++;
    }
    return 0;
}





// check content of command line arguments
void checkRMArg(char *port_num, size_t num_player, size_t num_hops){
    // check port number
    int num = atoi(port_num);
    if (num < 1024 || num > 65535) {
        cerr << "Error: port number should be between 1024 and 65535" << endl;
        exit(1);
    }

    // check number of players: greater than 1
    if (num_player < 2) {
        cerr << "Error: number of players should be greater than 1" << endl;
        exit(1);
    }

    // check number of hops: greater than or equal to 0 and less than or equal to 512
    if (num_hops < 0 || num_hops > 512) {
        cerr << "Error: number of hops should be greater than or equal to 0 and less than or equal to 512" << endl;
        exit(1);
    }

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
    char *port_num = argv[1];
    size_t num_players = atoi(argv[2]);
    size_t num_hops = atoi(argv[3]);

    // check the content of command line arguments
    checkRMArg(port_num, num_players, num_hops);
    
    // initialize the potato
    Potato potato(num_hops);

    // initialize the ringmaster
    Ringmaster *ringmaster = new Ringmaster(port_num, num_players, num_hops);
    // set up the ringmaster server
    Server * rm_server = new Server();
    rm_server->createSocket(port_num, false, *rm_server);

    // connect to the players
    ringmaster->connectToPlayers(*ringmaster, *rm_server);

    //create ring process
    ringmaster->createRing(*ringmaster, *rm_server);

    // launch the potato to the first random player
    ringmaster->launchPotato(potato);

    // print out the trace of the potato when get it back from the last player
    ringmaster->printTrace(potato);

    return 0;
}