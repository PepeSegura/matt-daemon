#include "daemon/Matt_daemon.hpp"

#include "Globals.hpp"
// Tintin_reporter reporter; // Will write in stdout
Tintin_reporter reporter("hola.log"); // If open fails will log in stderr


int main(void)
{
    Matt_daemon matt;
    std::cout << "Main" << std::endl;

    // reporter.debug("Main.cpp: this is DEBUG");
    // sleep(1);
    // reporter.info("Main.cpp: this is INFO");
    // sleep(1);
    // reporter.warning("Main.cpp: this is WARNING");
    // sleep(1);
    // reporter.error("Main.cpp: this is ERROR");
    reporter.info("PID: [" + std::to_string(getpid()) + "]");
    while (1)
    {
        reporter.info("sleeping...");
        sleep(1);
    }
    return (0);
}