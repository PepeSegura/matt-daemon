#include "Globals.hpp"

void signal_handler(int signum, siginfo_t *info, void *context)
{
	(void)info, (void)context;
	reporter.warning("Signal(" + std::to_string(signum) + ") received: " + strsignal(signum));
	if (signum == SIGINT || signum == SIGQUIT || signum == SIGABRT || signum == SIGTERM)
		end_program = 1;
}

void Matt_daemon:: handle_signals(void)
{
	struct sigaction	sa;

	sa.sa_flags = SA_SIGINFO;
	sa.sa_sigaction = signal_handler;
	for (int sig = 1; sig < NSIG; sig++)
	{
		if (sig != SIGKILL && sig != SIGSTOP)
			sigaction(sig, &sa, NULL);
	}
}

Matt_daemon:: Matt_daemon()
{
	reporter.info("Matt_daemon: Defaut constructor");
	this->handle_signals();
	this->healthcheck();
	this->create_daemon();
	this->create_server();
}

Matt_daemon:: Matt_daemon(const Matt_daemon& other)
{
	reporter.info("Matt_daemon: Copy constructor");
	*this = other;
}

Matt_daemon:: ~Matt_daemon()
{
	reporter.info("Matt_daemon: Destructor");
	unlink(LOCK_FILE);
}

Matt_daemon& Matt_daemon:: operator=(const Matt_daemon& other)
{
	reporter.info("Matt_daemon: = operand");
	if (this != &other)
		*this = other;
	return (*this);
}


void Matt_daemon:: healthcheck(void)
{
	// Check if calling user is root
	pid_t calling_process = geteuid();
	if (calling_process != 0)
	{
		reporter.error("Matt_daemon: program not running as root");
		exit(EXIT_FAILURE);
	}
	
	// Creating lockfile
	this->fd_lock = open(LOCK_FILE, O_CREAT | O_RDWR, 0666);
	if (this->fd_lock == -1)
	{
		reporter.error("Matt_daemon: Cannot create lock in \"/var/locks\"");
		exit(EXIT_FAILURE);
	}
	
	// Locking file, or exiting if it is already locked
	if (flock(this->fd_lock, LOCK_EX | LOCK_NB) == -1)
	{
		reporter.error("Matt_daemon: there is already running an instance of Matt_daemon");
		exit(EXIT_FAILURE);
	}
}

void Matt_daemon:: create_daemon(void)
{
	reporter.info("Matt_daemon: create_daemon()");
	pid_t pid;

	pid = fork();
	if (pid < 0) // Error
	{
		reporter.error("Matt_daemon: Error while creating the fork()");
		exit(EXIT_FAILURE);
	}
	if (pid > 0) // Parent
	{
		reporter.info("Matt_daemon: First Fork created sucessfully with pid: [" + std::to_string(pid) +  "]");
		reporter.info("Matt_daemon: Closing first parent");
		exit(EXIT_SUCCESS);
	}

	usleep(250 * 1000);
	// In the first child process
	if (setsid() < 0)
	{
		reporter.error("Matt_daemon: Failed to create session: " + std::string(strerror(errno)));
		exit(EXIT_FAILURE);
	}
	// Fork again to ensure the daemon is not a session leader
	pid = fork();
	if (pid < 0) // Errror
	{
		reporter.error("Matt_daemon: Error while creating the second fork: " + std::string(strerror(errno)));
		exit(EXIT_FAILURE);
	}
	if (pid > 0) // Parent
	{
		reporter.info("Matt_daemon: Second Fork created sucessfully with pid: [" + std::to_string(pid) +  "]");
		reporter.info("Matt_daemon: Closing second parent");
		exit(EXIT_SUCCESS);
	}
	usleep(250 * 1000);

	// In the second child process (daemon)
	// Change the working directory
	if (chdir("/") < 0)
	{
		reporter.error("Matt_daemon: Failed to change directory: " + std::string(strerror(errno)));
		exit(EXIT_FAILURE);
	}

	reporter.info("Redirecting stdin, stdout and stderr to \"/dev/null\"");
	int dev_null = open("/dev/null", O_RDWR);
	if (dev_null == -1)
	{
		reporter.error("Matt_daemon: Couldn't open \"/dev/null\": " + std::string(strerror(errno)));
		exit(EXIT_FAILURE);
	}

	// Redirect stdin, stdout, and stderr to /dev/null
	if (dup2(dev_null, STDIN_FILENO) == -1)
	{
		reporter.error("Matt_daemon: Failed to redirect stdin: " + std::string(strerror(errno)));
		close(dev_null);
		exit(EXIT_FAILURE);
	}

	if (dup2(dev_null, STDOUT_FILENO) == -1)
	{
		reporter.error("Matt_daemon: Failed to redirect stdout: " + std::string(strerror(errno)));
		close(dev_null);
		exit(EXIT_FAILURE);
	}

	if (dup2(dev_null, STDERR_FILENO) == -1)
	{
		reporter.error("Matt_daemon: Failed to redirect stderr: " + std::string(strerror(errno)));
		close(dev_null);
		exit(EXIT_FAILURE);
	}
	close(dev_null);
	reporter.info("Matt_daemon: daemon created!");
}
