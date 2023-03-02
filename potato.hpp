#include <iostream>
#include <cstddef>
#include <vector>
#include <ctime>


class Potato {
    private:
        size_t nhops; // number of hops
        std::vector <size_t> trace; // trace of the potato


    public:
        // constructor
        Potato(size_t nhops): nhops(nhops){};
        size_t getHops() const {
            return nhops;
        }
        // set the number of hops
        void setHops(size_t nhops) {
            this->nhops = nhops;
        }
        // get the trace of the potato
        std::vector <size_t> getTrace() const {
            return trace;
        }
        // add player to the trace
        void addTrace(size_t player) {
            trace.push_back(player);
        }
        // print out the trace of the potato
        void printTrace() const {
            std::cout << "Trace of potato: " << std::endl;
            for (size_t i = 0; i < trace.size(); i++) {
                std::cout << trace[i] << ",";
            }
            std::cout << std::endl;
        }

};

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


class Player {
    private:
        char *hostname;
        char *port_num;
        size_t player_id;
        int rm_sockfd;
        int prev_sockfd;
        int next_sockfd;


    public:
        // constructor 
        Player (){};

        // get the hostname
        char * getHostname() const {
            return hostname;
        }

        // get the port number
        char * getPortNum() const {
            return port_num;
        }
 

        // get the player id
        size_t getPlayerId() const {
            return player_id;
        }

        // set the player id
        void setPlayerId(size_t player_id) {
            this->player_id = player_id;
        }

        // set the hostname
        void setHostname(char *hostname) {
            this->hostname = hostname;
        }

        // set the port number
        void setPortNum(char *port_num) {
            this->port_num = port_num;
        }

        // check content of command line arguments
        int checkPlayerArg(const char *machine_name, char *port_num);

        // connect to the ringmaster
        int connectToRingmaster();

        // connect to neighbors
        int connectToNeighbors();

        // keep listening to ringmaster, left neighbor and right neighbor, and receive potato
        int receivePotato();

        // handle the potato, either pass it to neighbor or send it back to ringmaster
        int passPotato(Potato potato);


};

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

class Ringmaster {
    private:
        char *port_num;
        size_t num_players;
        size_t num_hops;
        Player *players;

    public:
        //constructor
        Ringmaster(char * port_num, size_t num_player, size_t num_hops): port_num(port_num), num_players(num_players), num_hops(num_hops) {
            // print out the ringmaster info
            std::cout << "Potato Rringmaster" << std::endl;
            std::cout << "Players = " << num_players << std::endl;
            std::cout << "Hops = " << num_hops << std::endl;
            };

        // get the port number
        char * getPortNum() const{
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
        
        // connect to the players
        int connectToPlayers();

        // create ring process
        int createRing();

        // lanch the potato to the first random player
        int launchPotato(Potato potato);

        // print out the trace of the potato when get it back from the last player
        void printTrace(Potato potato){
            potato.printTrace();
        }

};

        // check content of command line arguments
        void checkRMArg(char *port_num, size_t num_player, size_t num_hops);

