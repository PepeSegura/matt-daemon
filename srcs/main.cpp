#include "reporter/Tintin_reporter.hpp"

Tintin_reporter reporter("hola.log");

int main(void)
{
    std::cout << "Main" << std::endl;

    reporter.debug("HOLA");
    return (0);
}