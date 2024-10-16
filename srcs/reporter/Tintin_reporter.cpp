#include "reporter/Tintin_reporter.hpp"

Tintin_reporter:: Tintin_reporter()
{
    std::cout << "Tintin_reporter: Defaut constructor" << std::endl;

    _pretty_format = true;
    _output = &std::cout;
}

Tintin_reporter:: Tintin_reporter(std::string filename)
{
    std::cout << "Tintin_reporter: Filename constructor" << std::endl;

    _pretty_format = true;
    if (filename == "/dev/stdout")
        _output = &std::cout;
    else if (filename == "/dev/stderr")
        _output = &std::cerr;
    else
    {
        //file.open(filename.c_str(), std::ios::out | std::ios::app);
        file.open(filename.c_str(), std::ios::out);
        if (!file)
        {
            std::cerr << "Cannot open: [" << filename << "] using std::cerr as output" << std::endl;
            _output = &std::cerr;
            return ;
        }
        _output = &file;
        _pretty_format = false;
    }
}

Tintin_reporter:: Tintin_reporter(const Tintin_reporter& other)
{
    std::cout << "Tintin_reporter: Copy constructor" << std::endl;
    *this = other;
}

Tintin_reporter:: ~Tintin_reporter()
{
    std::cout << "Tintin_reporter: Destructor" << std::endl;
}

Tintin_reporter& Tintin_reporter:: operator=(const Tintin_reporter& other)
{
    std::cout << "Tintin_reporter: = operand" << std::endl;
    if (this != &other)
		*this = other;
    return (*this);
}

void Tintin_reporter:: print_time(void)
{
    std::time_t now = std::time(NULL);
    std::tm* local_time = std::localtime(&now);

    *(_output) << "[" <<  (_pretty_format ? GREY : "")
              << std::setw(2) << std::setfill('0') << local_time->tm_mday << "/"
              << std::setw(2) << std::setfill('0') << (local_time->tm_mon + 1) << "/"
              << (local_time->tm_year + 1900) << "-"
              << std::setw(2) << std::setfill('0') << local_time->tm_hour << ":"
              << std::setw(2) << std::setfill('0') << local_time->tm_min << ":"
              << std::setw(2) << std::setfill('0') << local_time->tm_sec
              << (_pretty_format ? RESET : "")
              << "]";
}

void Tintin_reporter:: print_mode(const std::string mode)
{
    std::string color = GREEN;

    if (mode == "DEBUG")
        color = YELLOW;
    else if (mode == "INFO")
        color = GREEN;
    else if (mode == "WARNING")
        color = ORANGE;
    else if (mode == "ERROR")
        color = RED;
    *(_output) << " [" << (_pretty_format ? color : "") << mode << (_pretty_format ? RESET : "") << "]" << std::string(8 - mode.length(), ' ') << "- ";
}

void    Tintin_reporter:: debug(const std::string log)
{
    print_time();
    print_mode("DEBUG");
    *(_output) << log << std::endl;
}

void    Tintin_reporter:: info(const std::string log)
{
    print_time();
    print_mode("INFO");
    *(_output) << log << std::endl;
}

void    Tintin_reporter:: warning(const std::string log)
{
    print_time();
    print_mode("WARNING");
    *(_output) << log << std::endl;
}

void    Tintin_reporter:: error(const std::string log)
{
    print_time();
    print_mode("ERROR");
    *(_output) << log << std::endl;
}