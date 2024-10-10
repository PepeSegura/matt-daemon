#pragma once

# include <iostream>

class Tintin_reporter
{
    public:
        Tintin_reporter(const std::string output_file);
        ~Tintin_reporter();
        void    debug(const std::string log);
        void    info(const std::string log);
        void    warning(const std::string log);
        void    error(const std::string log);
    
    private:
        Tintin_reporter();
        Tintin_reporter(const Tintin_reporter &other);
        Tintin_reporter& operator=(const Tintin_reporter &other);
        std::string _output_file;
};