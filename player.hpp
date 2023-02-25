/* 
The player side will be responsible for the following:
1. Establish three network socket connections for communication:
a. with the player to the left
b. with the player to the right
c. with the ringmaster
2. Keep listening to the three channels as “potato” can arrive on any of these three channels. Note that commands and important information may also be received from the ringmaster.
3. Properly handle everything received based on game rules.
*/

/*The player program is invoked as:
player <machine_name> <port_num>
(example: ./player vcm-xxxx.vm.duke.edu 1234)
*/

/*where machine_name is the machine name (e.g. login-teer-03.oit.duke.edu) where the ringmaster process is running and port_num is the port number given to the ringmaster process which it uses to open a socket for player connections. If there are N players, each player will have an ID of 0, 1, 2, to N-1.
A player’s ID and other information that each player will need to connect to their left and right neighbor can be provided by the ringmaster as part of setting up the game. The players are connected in the ring such that the left neighbor of player i is player i-1 and the right neighbor is player i+1. Player 0 is the right neighbor of player N-1, and Player N-1 is the left neighbor of player 0.
*/

#include "ringmaster.hpp"

class Player {
    private:
        const char *machine_name;
        const char *port_num;
        size_t player_id;
        Player *left;
        Player *right;
        
}