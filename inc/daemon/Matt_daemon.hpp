#pragma once

# include "reporter/Tintin_reporter.hpp"
# include "daemon/Server.hpp"

class Matt_daemon
{
    # define LOCK_FILE "/var/lock/Matt_daemon.lock"
    public:
        Matt_daemon();
        ~Matt_daemon();
    
    private:
        int fd_lock;

        Matt_daemon(const Matt_daemon &other);
        Matt_daemon& operator=(const Matt_daemon &other);
        
        void healthcheck(void);
        void create_daemon(void);
        void handle_signals(void);

        void create_server() {
            Server(); // Create Restricted instance
        }
};
