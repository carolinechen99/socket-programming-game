#include <iostream>
#include <cstddef>
#include <vector>
#include <ctime>
#include <algorithm>

#include "socketutils.hpp"


class Potato {
    public:
        size_t nhops; // number of hops
        size_t trace[1024]; // trace of the potato
        size_t fix_hop; // fixed hop


    public:
        // constructor
        Potato(): nhops(0), fix_hop(0) {
            memset(trace, 0, sizeof(trace));
    }

        Potato(size_t nhops): nhops(nhops), fix_hop(nhops) {
            memset(trace, 0, sizeof(trace));
        }

        size_t getHops() const {
            return this->nhops;
        }
        size_t getFixHop() const {
            return this->fix_hop;
        }

        // set the number of hops
        void setHops(size_t nhops) {
            this->nhops = nhops;
        }
        // add player to the trace
        void addTrace(size_t player_id) {
            this->trace[this->fix_hop - this->nhops] = player_id;
            
        }

        // print out the trace of the potato
        void printTrace() const {
            if (this->fix_hop == 0) {
                return;
            }
            std::cout << "Trace of potato: " << std::endl;
            for (size_t i = 0; i < this->fix_hop-1; i++) {
                std::cout << this->trace[i] << ",";
            }
            std::cout << this->trace[fix_hop-1];
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
    public:
        char hostname[256];
        char port_num[100];
        size_t player_id;
        int master_sockfd;
        int prev_sockfd;
        int next_sockfd;
        int num_players;


    public:
        // constructor 
        Player (){
            player_id = 0;
            master_sockfd = -1;
            prev_sockfd = -1;
            next_sockfd = -1;
            num_players = 0;
            memset(hostname, 0, sizeof(hostname));
            memset(port_num, 0, sizeof(port_num));
        }
        
        // get master socket file descriptor
        int getMasterSockfd() const {
            return master_sockfd;
        }

        // get previous socket file descriptor
        int getPrevSockfd() const {
            return prev_sockfd;
        }

        // get next socket file descriptor
        int getNextSockfd() const {
            return next_sockfd;
        }

        // get the number of players
        int getNumPlayers() const {
            return num_players;
        }


        // get the hostname
        const char* getHostname() const {
            return hostname;
        }

        // get the port number
        const char* getPortNum() const {
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
            strcpy(this->hostname, hostname);
        }

        // set the port number
        void setPortNum(char *port_num) {
            strcpy(this->port_num, port_num);
        }

        // set master socket file descriptor
        void setMasterSockfd(int master_sockfd) {
            this->master_sockfd = master_sockfd;
        }

        // set previous socket file descriptor
        void setPrevSockfd(int prev_sockfd) {
            this->prev_sockfd = prev_sockfd;
        }

        // set next socket file descriptor
        void setNextSockfd(int next_sockfd) {
            this->next_sockfd = next_sockfd;
        }

        // set the number of players
        void setNumPlayers(int num_players) {
            this->num_players = num_players;
        }

        // check content of command line arguments
        int checkPlayerArg(const char *machine_name, const char *port_num);

        // connect to the ringmaster
        int connectToRingmaster(const char *machine_name, const char *master_port, Server &player_server);

        // connect to next player
        int connectNextPlayer();

        // accept connection from previous player
        int acceptPrevPlayer(Server &player_server);

        // connect to neighbor players
        int connectToNeighbors(Server &player_server);

        // keep listening to ringmaster, left neighbor and right neighbor, and receive potato
        int handlePotato();


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
    public:
        char port_num [100];
        size_t num_players;
        size_t num_hops;
        std::vector <int> player_fds;
        std::vector <std::string> player_hostnames;
        std::vector <std::string> player_port_nums;


    public:
        //constructor
        Ringmaster(const char * port_num, size_t num_player, size_t num_hops): num_players(num_players), num_hops(num_hops) {
            memset(this->port_num, 0, sizeof(this->port_num));
            strcpy(this->port_num, port_num);
            }

        // get player fds
        std::vector <int> getPlayerFds() const {
            return player_fds;
        }

        // get player hostnames
        std::vector <std::string> getPlayerHostnames() const {
            return player_hostnames;
        }

        // get player port numbers
        std::vector <std::string> getPlayerPortNums() const {
            return player_port_nums;
        }

        // get the port number
        const char* getPortNum() const {
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

        // set the port number
        void setPortNum(const char *port_num) {
            strcpy(this->port_num, port_num);
        }

        // set the number of players
        void setNumPlayers(size_t num_players) {
            this->num_players = num_players;
        }

        // add player fds
        void addPlayerFd(int player_fd) {
            this->player_fds.push_back(player_fd);
        }

        // add player hostnames
        void addPlayerHostname(std::string player_hostname) {
            this->player_hostnames.push_back(player_hostname);
        }

        // add player port numbers
        void addPlayerPortNum(std::string player_port_num) {
            player_port_nums.push_back(player_port_num);
        }
        
        // connect to the players
        int connectToPlayers(Server &master_server);

        // create ring process
        int createRing(Server &server);

        // lanch the potato to the first random player
        int launchPotato(size_t num_hops);
        // receive potato from the last player
        int receivePotato();

        // print out the trace of the potato when get it back from the last player
        void printTrace(Potato &potato){
            potato.printTrace();
        }

        // shut down the game
        int shutDownGame();

};

        // check content of command line arguments
        int checkRMArg(const char *port_num, size_t num_player, size_t num_hops);

