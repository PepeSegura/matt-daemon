#include "reporter/Tintin_reporter.hpp"

Tintin_reporter:: Tintin_reporter()
{
    std::cout << "Defaut constructor" << std::endl;
	_output_file = "Unsetted";
}

Tintin_reporter:: Tintin_reporter(std::string output_file) : _output_file(output_file)
{
    std::cout << "Filename constructor" << std::endl;
}

Tintin_reporter:: Tintin_reporter(const Tintin_reporter& other)
{
    std::cout << "Copy constructor" << std::endl;
    *this = other;
}

Tintin_reporter:: ~Tintin_reporter()
{
    std::cout << "Destructor" << std::endl;
}

Tintin_reporter& Tintin_reporter:: operator=(const Tintin_reporter& other)
{
    std::cout << "= operand" << std::endl;
    if (this != &other)
		*this = other;
    return (*this);
}


void    Tintin_reporter:: debug(const std::string log)
{
    std::cout << "DEBUG: " << log << std::endl;
    std::cout << "IMPLEMENT FORMAT: [ DD / MM / YYYY - HH : MM : SS ]\n";
}

void    Tintin_reporter:: info(const std::string log)
{
    std::cout << "INFO: " << log << std::endl; 
}

void    Tintin_reporter:: warning(const std::string log)
{
    std::cout << "WARNING: " << log << std::endl;
}

void    Tintin_reporter:: error(const std::string log)
{
    std::cout << "ERROR: " << log << std::endl;
}