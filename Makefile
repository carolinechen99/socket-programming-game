TARGETS = ringmaster player
all: $(TARGETS)
clean:
	rm -f $(TARGETS)
ringmaster: ringmaster.cpp socketutils.cpp socketutils.hpp potato.hpp
	g++ -g -o ringmaster ringmaster.cpp socketutils.cpp socketutils.hpp potato.hpp 
player: player.cpp socketutils.cpp socketutils.hpp potato.hpp 
	g++ -g -o player player.cpp socketutils.cpp socketutils.hpp potato.hpp 
