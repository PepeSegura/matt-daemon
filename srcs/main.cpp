#include "reporter/Tintin_reporter.hpp"

// Tintin_reporter reporter; // Will write in stdout
Tintin_reporter reporter("hola.log"); // If open fails will log in stderr


int main(void)
{
    std::cout << "Main" << std::endl;

    reporter.debug("this is DEBUG");
    sleep(1);
    reporter.info("this is INFO");
    sleep(1);
    reporter.warning("this is WARNING");
    sleep(1);
    reporter.error("this is ERROR");
    return (0);
}