#include "reporter/Tintin_reporter.hpp"



#include <sys/stat.h>  // For mkdir
#include <fcntl.h>     // For open
#include <unistd.h>    // For close
#include <errno.h>     // For errno and strerror
#include <cstring>     // For strerror
#include <iostream>    // For std::cerr
#include <string>      // For std::string

void Tintin_reporter::open_file(void)
{
    auto create_directories = [](const std::string& path) -> int
    {
        size_t pos = 0;
        std::string current_path;

        while ((pos = path.find('/', pos)) != std::string::npos)
        {
            current_path = path.substr(0, pos++);
            if (!current_path.empty() && mkdir(current_path.c_str(), 0755) && errno != EEXIST)
            {
                std::cerr << "Error: Failed to create directory " << current_path << ": " << strerror(errno) << std::endl;
                return -1;
            }
        }
        if (mkdir(path.c_str(), 0755) && errno != EEXIST)
        {
            std::cerr << "Error: Failed to create directory " << path << ": " << strerror(errno) << std::endl;
            return -1;
        }
        return 0;
    };

    // Extract the directory from _filename
    size_t dir_pos = _filename.find_last_of('/');
    if (dir_pos != std::string::npos)
    {
        std::string dir_path = _filename.substr(0, dir_pos);
        if (create_directories(dir_path) == -1)
        {
            std::cerr << "Cannot create directory for: [" << _filename << "] using std::cerr as output" << std::endl;
            _fd = STDERR_FILENO;
            return ;
        }
    }

    _fd = open(_filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
    if (_fd == -1)
    {
        std::cerr << "Cannot open: [" << _filename << "] using std::cerr as output" << std::endl;
        _fd = STDERR_FILENO;
        return ;
    }
}


// static bool file_has_rigths(const std::string &filename)
// {
//     struct stat fileStat;
//     if (stat(filename.c_str(), &fileStat) < 0)
//         return false;
//     if ((fileStat.st_mode & S_IRUSR) && (fileStat.st_mode & S_IWUSR))
//         return true;
//     return false;
// }

static bool file_has_rigths(const std::string &filename, int fd)
{
    struct stat fileStat;
    struct stat fdStat;

    // Get the status of the file from the filename
    if (stat(filename.c_str(), &fileStat) < 0)
        return false;

    // Get the status of the file from the file descriptor
    if (fstat(fd, &fdStat) < 0)
        return false;

    if (fileStat.st_dev == fdStat.st_dev && fileStat.st_ino == fdStat.st_ino)
    {
        // Check read and write permissions for the file
        if ((fileStat.st_mode & S_IRUSR) && (fileStat.st_mode & S_IWUSR))
            return true;
    }
    return false;
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
        if (file_has_rigths(_filename, this->_fd) == false)
        {
            unlink(_filename.c_str());
            remove(_filename.c_str());
        }
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

    if (mode == "DEBUG")
        color = YELLOW;
    else if (mode == "INFO")
        color = GREEN;
    else if (mode == "WARNING")
        color = ORANGE;
    else if (mode == "ERROR")
        color = RED;

    std::stringstream buffer;
    buffer << " [" << (_pretty_format ? color : "") << mode << (_pretty_format ? RESET : "") << "]" << std::string(8 - mode.length(), ' ') << "- ";

    this->_log_msg += buffer.str();
}

void Tintin_reporter:: print_buffer(void)
{
    if (file_has_rigths(_filename, this->_fd) == false)
    {
        std::cerr << "File no longer valid, recreating..." << std::endl;
        close(this->_fd);
        unlink(_filename.c_str());
        remove(_filename.c_str());
        open_file();
        std::string old_buffer = this->_log_msg;
        this->_log_msg = "";
        error("Tintin_reporter: There was a problem with \"" + _filename + "\". Recreating file...");
        this->_log_msg = old_buffer;
    }
    write(this->_fd, this->_log_msg.c_str(), this->_log_msg.size());
    this->_log_msg = "";
}

void    Tintin_reporter:: debug(const std::string log)
{
    print_time();
    print_mode("DEBUG");

    this->_log_msg += log + "\n";
    print_buffer();
}

void    Tintin_reporter:: info(const std::string log)
{
    print_time();
    print_mode("INFO");

    this->_log_msg += log + "\n";
    print_buffer();
}

void    Tintin_reporter:: warning(const std::string log)
{
    print_time();
    print_mode("WARNING");

    this->_log_msg += log + "\n";
    print_buffer();
}

void    Tintin_reporter:: error(const std::string log)
{
    print_time();
    print_mode("ERROR");

    this->_log_msg += log + "\n";
    print_buffer();
}