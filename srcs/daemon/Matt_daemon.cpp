#include "Globals.hpp"

Matt_daemon:: Matt_daemon()
{
    std::cout << "Matt_daemon: Defaut constructor" << std::endl;
    this->healthcheck();
    this->create_daemon();
}

Matt_daemon:: Matt_daemon(const Matt_daemon& other)
{
    std::cout << "Matt_daemon: Copy constructor" << std::endl;
    *this = other;
}

Matt_daemon:: ~Matt_daemon()
{
    std::cout << "Matt_daemon: Destructor" << std::endl;
}

Matt_daemon& Matt_daemon:: operator=(const Matt_daemon& other)
{
    std::cout << "Matt_daemon: = operand" << std::endl;
    if (this != &other)
		*this = other;
    return (*this);
}

void Matt_daemon:: healthcheck(void)
{
    // Check if calling user is root
    pid_t calling_process = geteuid();
    if (calling_process != 0)
        reporter.error("Matt_daemon: program not running as root");
    
    // Creating lockfile
    int fd_lock = open("/var/lock/Matt_daemon.lock", O_CREAT | O_RDWR, 0666);
    if (fd_lock == -1)
        reporter.error("Matt_daemon: Cannot create lock in \"/var/locks\"");
    
    // Locking file, or exiting if it is already locked
    if (flock(fd_lock, LOCK_EX | LOCK_NB) == -1)
        reporter.error("Matt_daemon: there is already running an instance of Matt_daemon");

    // Release the lock by closing the file descriptor
    // close(fd_lock);
    // reporter.info("Lock released");
}

void Matt_daemon:: create_daemon(void)
{
    reporter.info("Matt_daemon: create_daemon()");
    pid_t pid;

    pid = fork();
    if (pid < 0)
    {
        reporter.error("Matt_daemon: Error while creating the fork()");
    }
    if (pid > 0)
    {
        reporter.info("Matt_daemon: Fork created sucessfully with pid: [" + std::to_string(pid) +  "]");
        reporter.info("Matt_daemon: Closing first parent");
        exit(EXIT_SUCCESS);
    }
    if (setsid() < 0)
    {
        reporter.warning("Matt_daemon: Failed to create session: " + std::string(strerror(errno)));
    }
    // Fork again to ensure the daemon is not a session leader
    pid = fork();
    if (pid < 0) {
        reporter.error("Matt_daemon: Error while creating the second fork: " + std::string(strerror(errno)));
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // Change the working directory
    if (chdir("/") < 0) {
        reporter.error("Matt_daemon: Failed to change directory: " + std::string(strerror(errno)));
    }

    reporter.info("Redirecting stdin, stdout and stderr to \"/dev/null\"");
    int dev_null = open("/dev/null", O_RDWR);
    if (dev_null == -1)
    {
        reporter.error("Matt_daemon: Couldn't open \"/dev/null\": " + std::string(strerror(errno)));
    }

    // Redirect stdin, stdout, stderr to /dev/null
    dup2(dev_null, STDIN_FILENO);
    dup2(dev_null, STDOUT_FILENO);
    dup2(dev_null, STDERR_FILENO);

    close(dev_null);

    reporter.info("Matt_daemon: daemon created!");
}
