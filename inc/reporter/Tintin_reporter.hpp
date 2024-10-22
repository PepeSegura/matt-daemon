#pragma once

# include <iostream>
# include <fstream>
# include <ctime>
# include <iomanip>
# include <unistd.h>
# include <fcntl.h>
# include <sys/stat.h>

# define RESET  "\033[0m"
# define GREY   "\033[38;5;240m"
# define GREEN  "\033[32m"
# define YELLOW "\033[33m"
# define ORANGE "\033[38;5;214m"
# define RED    "\033[31m"


class Tintin_reporter
{
    public:
        Tintin_reporter();
        Tintin_reporter(const std::string filename);
        ~Tintin_reporter();

        void    debug(const std::string log);
        void    info(const std::string log);
        void    warning(const std::string log);
        void    error(const std::string log);
    
    private:
        std::ostream*   _output;
        std::ofstream   _file;
        std::string     _filename;
        std::string     _log_msg;
        int             _fd;
        bool            _pretty_format;


        void print_time(void);
        void print_mode(const std::string mode);
        void open_file(void);
        void print_buffer(void);

        Tintin_reporter(const Tintin_reporter &other);
        Tintin_reporter& operator=(const Tintin_reporter &other);
};