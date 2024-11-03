#pragma once


# include <sys/stat.h>  // For mkdir
# include <fcntl.h>     // For open
# include <unistd.h>    // For close
# include <errno.h>     // For errno and strerror
# include <cstring>     // For strerror
# include <iostream>    // For std::cerr
# include <string>      // For std::string
# include <ctime>       // For std::time and std::localtime

# include <fstream>
# include <iomanip>

# define RESET  "\033[0m"
# define GREY   "\033[38;5;240m"
# define GREEN  "\033[32m"
# define YELLOW "\033[33m"
# define ORANGE "\033[38;5;214m"
# define RED    "\033[31m"

# define MAX_MSGS 4

constexpr size_t MAX_FILE_SIZE = 10 * 1024;

class Tintin_reporter
{
    public:
        Tintin_reporter(const std::string filename);
        ~Tintin_reporter();

        void    debug(const std::string log);
        void    info(const std::string log);
        void    warning(const std::string log);
        void    error(const std::string log);
    
    private:
        std::string     _filename;
        std::string     _backup_dir;

        std::string     _log_msg;
        int             _fd;
        bool            _pretty_format;


        void print_time(void);
        void print_mode(const std::string mode);
        void open_file(void);
        void backup_file(void);
        void print_buffer(void);

        Tintin_reporter();
        Tintin_reporter(const Tintin_reporter &other);
        Tintin_reporter& operator=(const Tintin_reporter &other);
};