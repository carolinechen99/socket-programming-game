/* Ringmaster will be responsible for:
1. Establish N network socket connections with N number of players and provide relevant
information to each player (see Communication Machanism section below for details)
2. Create a “potato” object as described above
3. Randomly select a player and send the “potato” to the selected player
4. At the end of the game (when the ringmaster receive the potato from the player who is
“it”), print a trace of the potato to the screen
5. Shut the game down by sending a message to each player
*/

/*The server program is invoked as:
ringmaster <port_num> <num_players> <num_hops> (example: ./ringmaster 1234 3 100)*/

/*Sample Ringmaster Output:
Potato Ringmaster
Players = 3
Hops = 200
Player 1 is ready to play
Player 0 is ready to play
Player 2 is ready to play
Ready to start the game, sending potato to player 2 Trace of potato:
2,1,2,0,2,1,0,2,...*/

//include potato.hpp
#include "potato.hpp"

class Ringmaster {
    private:
        const char *port_num;
        size_t num_players;
        size_t num_hops;
        Potato potato;

    public:
        //constructor
        Ringmaster(const char *port_num, size_t num_player, size_t num_hops): port_num(port_num), num_players(num_players), num_hops(num_hops), potato(num_hops) {
            // print out the ringmaster info
            std::cout << "Potato Rringmaster" << std::endl;
            std::cout << "Players = " << num_players << std::endl;
            std::cout << "Hops = " << num_hops << std::endl;
            };

        // get the port number
        const char *getPortNum() const{
            return port_num;
        }

        // get the number of players
        size_t getNumPlayers() const{
            return num_players;
        }

        // get the number of hops
        size_t getNumHops() const{
            return num_hops;
        }
        

};
