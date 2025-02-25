#include "MattDaemon.hpp"
#include "Tintin_reporter.hpp"


int main() {
    if (getuid() != 0) {
        std::cerr << "Error: MattDaemon must be run as root!" << std::endl;
        return EXIT_FAILURE;
    }
   
    MattDaemon daemon;
    daemon.run();
    return 0;
}
