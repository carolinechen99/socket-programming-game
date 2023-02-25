#include <iostream>
#include <cstddef>
#include <vector>


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
        std::vector <size_t> getTrace() const(){
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