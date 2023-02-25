#include "potato.hpp"

using namespace std;

//connect to the ringmaster
void Player::connectToRingmaster(){}


// check the content of command line arguments
void checkPlayerArg(const char *machine_name, int port_num) {
    // check port number
    if (port_num < 1024 || port_num > 65535) {
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
    const char *machine_name = argv[1];
    int port_num = atoi(argv[2]);

    // check the content of command line arguments
    checkPlayerArg(machine_name, port_num);

    // initialize the player
    Player player(machine_name, port_num);

    // connect to the ringmaster
    player.connectToRingmaster();

}