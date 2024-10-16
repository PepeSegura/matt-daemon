#pragma once

# include "reporter/Tintin_reporter.hpp"

# include <iostream>
# include <fstream>
# include <ctime>
# include <iomanip>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/stat.h>
# include <sys/types.h>
# include <fcntl.h>
# include <cerrno>
# include <cstring>

class Matt_daemon
{
    public:
        Matt_daemon();
        ~Matt_daemon();

    
    private:
        Matt_daemon(const Matt_daemon &other);
        Matt_daemon& operator=(const Matt_daemon &other);
        
        void create_daemon(void);
};