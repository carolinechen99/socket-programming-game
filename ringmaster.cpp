#include "potato.hpp"

using namespace std;

// connect to the players
void Ringmaster::connectToPlayers(){}

// lanch the potato to the first random player
void Ringmaster::launchPotato(Potato potato){}


// check content of command line arguments
void checkRMArg(int port_num, size_t num_player, size_t num_hops){
    // check port number
    if (port_num < 1024 || port_num > 65535) {
        cerr << "Error: port number should be between 1024 and 65535" << endl;
        exit(1);
    }

    // check number of players: greater than 1
    if (num_player < 2) {
        cerr << "Error: number of players should be greater than 1" << endl;
        exit(1);
    }

    // check number of hops: greater than or equal to 0
    if (num_hops < 0) {
        cerr << "Error: number of hops should be greater than or equal to 0" << endl;
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
    int port_num = atoi(argv[1]);
    size_t num_players = atoi(argv[2]);
    size_t num_hops = atoi(argv[3]);

    // check the content of command line arguments
    checkRMArg(port_num, num_players, num_hops);
    
    // initialize the potato
    Potato potato(num_hops);

    // initialize the ringmaster
    Ringmaster ringmaster(port_num, num_players, num_hops);

    // print out the ringmaster info
    cout << "Potato Ringmaster" << endl;
    cout << "Players = " << ringmaster.getNumPlayers() << endl;
    cout << "Hops = " << ringmaster.getNumHops() << endl;

    // connect to the players
    ringmaster.connectToPlayers();

    // launch the potato to the first random player
    ringmaster.launchPotato(potato);

    // print out the trace of the potato when get it back from the last player
    ringmaster.printTrace(potato);

    return 0;
}