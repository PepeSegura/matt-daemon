#include "Globals.hpp"

Matt_daemon:: Matt_daemon()
{
    std::cout << "Matt_daemon: Defaut constructor" << std::endl;
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

void Matt_daemon:: create_daemon(void)
{
    reporter.info("Matt_daemon: create_daemon()");
    pid_t pid;

    pid = fork();
    if (pid < 0)
    {
        reporter.error("Matt_daemon: Error while creating the fork()");
        exit(EXIT_FAILURE);
    }
    if (pid > 0)
    {
        reporter.info("Matt_daemon: Fork created sucessfully with pid: [" + std::to_string(pid) +  "]");
    }
    if (pid == 0)
    {
        reporter.info("Matt_daemon: Closing first parent");
        exit(EXIT_SUCCESS);
    }
    if (setsid() < 0)
    {
        reporter.error("Matt_daemon: Failed to create session: " + std::string(strerror(errno)));
    }
    // Fork again to ensure the daemon is not a session leader
    pid = fork();
    if (pid < 0) {
        reporter.error("Matt_daemon: Error while creating the second fork: " + std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // Change the working directory
    if (chdir("/") < 0) {
        reporter.error("Matt_daemon: Failed to change directory: " + std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    }

    int dev_null = open("/dev/null", O_RDWR);
    if (dev_null == -1)
    {
        reporter.warning("Matt_daemon: Couldn't open \"/dev/null\": " + std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    }

    // Redirect stdin, stdout, stderr to /dev/null
    dup2(dev_null, STDIN_FILENO);
    dup2(dev_null, STDOUT_FILENO);
    dup2(dev_null, STDERR_FILENO);

    close(dev_null);

    reporter.info("Matt_daemon: daemon created!");
}
