#pragma once

# include "Globals.hpp"

class Matt_daemon;

class Server
{
    public:
        ~Server();
    
    private:
        Server();
        friend class Matt_daemon;

        Server(const Server &other);
        Server& operator=(const Server &other);
};