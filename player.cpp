#include "potato.hpp"
#include "socketutils.hpp"

using namespace std;

//connect to the ringmaster
int Player::connectToRingmaster(){
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
    Server * player_server = new Server();
    player_server->createSocket(NULL, true);

    // connect to the ringmaster
    player->connectToRingmaster();

}