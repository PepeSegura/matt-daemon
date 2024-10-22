#include "reporter/Tintin_reporter.hpp"

Tintin_reporter:: Tintin_reporter()
{
    std::cout << "Tintin_reporter: Defaut constructor" << std::endl;

    _pretty_format = true;
    _fd = STDOUT_FILENO;
}

void Tintin_reporter:: open_file(void)
{
    _fd = open(_filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
    if (_fd == -1)
    {
        std::cerr << "Cannot open: [" << _filename << "] using std::cerr as output" << std::endl;
        _fd = STDERR_FILENO;
        exit(1);
        return ;
    }
}

Tintin_reporter:: Tintin_reporter(std::string filename) : _filename(filename)
{
    std::cout << "Tintin_reporter: Filename constructor" << std::endl;

    _pretty_format = true;
    if (_filename == "/dev/stdout")
        _fd = STDOUT_FILENO;
    else if (_filename == "/dev/stderr")
        _fd = STDERR_FILENO;
    else
    {
        int ret;
        if ((ret = access(_filename.c_str(), F_OK | R_OK | W_OK)) == -1)
        {
            std::cerr << "File does not exist or lacks permissions (read/write): " << filename << std::endl;
            unlink(_filename.c_str());
            if (remove(_filename.c_str()) == 0) {
                std::cerr << "File: " << _filename << "deleted successfully." << std::endl;
            } else {
                std::cerr << "Error deleting file: " << _filename  << std::endl;
                // Print an error message with details if deletion fails
                perror("Error deleting the file");
            }
        }
        std::cerr << "ERR: " << ret << std::endl;
        open_file();
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

    std::stringstream buffer;
    buffer << "[" <<  (_pretty_format ? GREY : "")
              << std::setw(2) << std::setfill('0') << local_time->tm_mday << "/"
              << std::setw(2) << std::setfill('0') << (local_time->tm_mon + 1) << "/"
              << (local_time->tm_year + 1900) << "-"
              << std::setw(2) << std::setfill('0') << local_time->tm_hour << ":"
              << std::setw(2) << std::setfill('0') << local_time->tm_min << ":"
              << std::setw(2) << std::setfill('0') << local_time->tm_sec
              << (_pretty_format ? RESET : "")
              << "]";

    this->_log_msg += buffer.str();
}

void Tintin_reporter:: print_mode(const std::string mode)
{
    std::string color = GREEN;
    std::stringstream buffer;

    if (mode == "DEBUG")
        color = YELLOW;
    else if (mode == "INFO")
        color = GREEN;
    else if (mode == "WARNING")
        color = ORANGE;
    else if (mode == "ERROR")
        color = RED;
    buffer << " [" << (_pretty_format ? color : "") << mode << (_pretty_format ? RESET : "") << "]" << std::string(8 - mode.length(), ' ') << "- ";

    this->_log_msg += buffer.str();
}

void Tintin_reporter:: print_buffer(void)
{
    if (access(_filename.c_str(), F_OK | R_OK | W_OK) == -1)
    {
        std::cerr << "File no longer valid, recreating..." << std::endl;
        close(this->_fd);
        open_file();
    }
    write(this->_fd, this->_log_msg.c_str(), this->_log_msg.size());
    this->_log_msg = "";
}

void    Tintin_reporter:: debug(const std::string log)
{
    print_time();
    print_mode("DEBUG");

    std::stringstream buffer;
    buffer << log << std::endl;
    this->_log_msg += buffer.str();
    print_buffer();
}

void    Tintin_reporter:: info(const std::string log)
{
    print_time();
    print_mode("INFO");

    std::stringstream buffer;
    buffer << log << std::endl;
    this->_log_msg += buffer.str();
    print_buffer();
}

void    Tintin_reporter:: warning(const std::string log)
{
    print_time();
    print_mode("WARNING");
    std::stringstream buffer;
    buffer << log << std::endl;
    this->_log_msg += buffer.str();
    print_buffer();
}

void    Tintin_reporter:: error(const std::string log)
{
    print_time();
    print_mode("ERROR");
    std::stringstream buffer;
    buffer << log << std::endl;
    this->_log_msg += buffer.str();
    print_buffer();
}