#pragma once

#ifndef GLOBALS_HPP
#define GLOBALS_HPP

# include <iostream>
# include <iomanip>
# include <fstream>
# include <ctime>
# include <cerrno>
# include <cstring>
# include <csignal>

# include <sys/stat.h>
# include <sys/types.h>
# include <sys/file.h>
# include <sys/socket.h>
# include <netinet/in.h>

# include <stdlib.h>
# include <stdio.h>
# include <fcntl.h>
# include <unistd.h>

# include <map>
# include <vector>

# include "daemon/Matt_daemon.hpp"

/* Globals */
extern Tintin_reporter reporter;
extern volatile sig_atomic_t end_program;

#endif
