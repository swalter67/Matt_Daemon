#include "MattDaemon.hpp"
#include "Tintin_reporter.hpp"

#include <stdexcept>

int main()
{
    try
    {
        if (getuid() != 0)
            throw std::runtime_error("Error: MattDaemon must be run as root!");
        MattDaemon daemon;
        daemon.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
