C++ 11 supported

Player id assignment: 
    - server assigns as soon as player connects

Player ready print:
    - connect to server, and then connect to neighbors, not same as id assignment

Player number:
    - no need to open more than 1000 clients to play the game

Server/Client: 
    - only matter when establish connection, after that, server and client are the same
    - set the rule myself

Game output lines:
    - n hops -> n-1 potatoes, 1 it

Player machine name
    - host name, use gethostname()?

Potota == 0:
    - create ring process, and immediately shutdown the game
    - do not print trace

use hton to manipulate int when sending and receiving

Process Desripiton:
    - Ringmaster initializes the game
    - hops(potato), numPlayers(players needed to be created), set up the socket

    - Set up players' sockets according to the number of players
        # Server * player-server = new

    - Player connect() to the ringmaster, 
        receive() id and numPlayers from ringmaster, and print `Connected as player x out of n total players`, send() machine_name and port_num to ringmaster
    ;
        # Player::connectToRingmaster();

    - ringmaster receive() machine_name and port_num from player, create ring process,  send() next player's machine_name and port_num to player
    ;
        # Ringmaster::createRing();
    
    - Player receive() next player's machine_name and port_num from ringmaster, connect() to next player, accept() connection from previous player, send() "ready" to ringmaster to indicate ready to play
    ;
        # Player::connectToNeighbors();
    - Ringmaster receive() ready from players, and print `Player x is ready to play`, when all players are ready, send() potato to random player, sending potato to player x`, random player receive() potato
    ;
        # Ringmaster::launchPotato(potato);
    -------In loop-------
    - random player listen() to ring master, previous player, and next player, and possibly receive() potato from them, add player id to trace
        # Player::receivePotato();
    - if nhops>0, player send() potato to neighbors randomly, print `Sending potato to y`, 
      if nhops == 0, player send() potato to ringmaster, print `I'm it`, ringmaster receive() potato, print `Trace of potato: x y z ...`
    ;
