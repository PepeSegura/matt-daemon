#include "Globals.hpp"

constexpr int PORT = 4242;
constexpr int MAX_CLIENTS = 3;

#include <memory>
#define MAGIC_NBR 250000

static void execute_command(const std::string &command, int client_socket)
{
    std::array<char, 128> buffer;
    std::string full_command = "timeout 2 bash -c '" + command + "' 2>&1";
    std::shared_ptr<FILE> pipe(popen(full_command.c_str(), "r"), pclose);

    if (!pipe)
    {
        reporter.error("popen() failed!");
        end_program = 1;
    }

    int total_written = 0;
    while (total_written < MAGIC_NBR && fgets(buffer.data(), 128, pipe.get()) != nullptr)
    {
        total_written += buffer.size();
        send(client_socket, buffer.data(), strlen(buffer.data()), 0);
    }

}

static void parse_message(std::string message, int client_socket)
{
    (void)client_socket;
    if (message == "quit")
    {
        reporter.info("Matt_daemon: quit received, closing server.");
        end_program = 1;
    }
    # ifdef BONUS
    else if (message.find("shell: ") == 0)
    {
        std::string command = message.substr(7);
        reporter.info("Matt_daemon: Command received: " + command);

        // Execute the command and get the output
        execute_command(command, client_socket);
    }
    #endif
    else
        reporter.info("Matt_daemon: Message received: " + message);
}

Server:: Server()
{
    reporter.info("Matt_daemon: Creating Server.");
    reporter.info("Matt_daemon: Server created with PID: " + std::to_string(getpid()));

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) // Create socket
    {
        reporter.error("Syscall: Socket error: " + std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) // Set socket options
    {
        reporter.error("Syscall: Setsockopt error: " + std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        reporter.error("Syscall: Bind error: " + std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0)
    {
        reporter.error("Syscall: Listen error: " + std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    }

    fd_set readfds;

    reporter.debug("Matt_daemon: Server listening on port: " + std::to_string(PORT));
    while (end_program == 0)
    {
        struct timeval  tv = {tv.tv_sec = 1, tv.tv_usec = 0};

        // clear read_set
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int max_sd = server_fd;

        // add client sockets to the set
        for (auto it = this->clients.begin(); it != this->clients.end(); ++it)
        {
            int client_socket = it->first;
            FD_SET(client_socket, &readfds);
            if (client_socket > max_sd)
                max_sd = client_socket;
        }

        // Retry select() if interrupted by a signal
        int activity;
        do
        {
            activity = select(max_sd + 1, &readfds, nullptr, nullptr, &tv);
        } while (activity < 0 && errno == EINTR);

        if (activity < 0)
        {
            reporter.error("Syscall: Select error: " + std::string(strerror(errno)));
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(server_fd, &readfds))
        {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) // accept new connection
            {
                reporter.error("Syscall: Accept error: " + std::string(strerror(errno)));
                exit(EXIT_FAILURE);
            }
            if (this->clients.size() == MAX_CLIENTS)
            {
                std::string error_message = "Matt_daemon: Can't accept the connection, max number of clients reached.";
                reporter.warning(error_message);
                send(new_socket, (error_message + "\n").c_str(), error_message.length() + 1, 0);
                close(new_socket);
                continue ;
            }
            // add new socket to clients map
            reporter.debug("Matt_daemon: New connection accepted.");
            this->clients[new_socket] = "";
        }

        // Check for activity on client sockets
        for (auto it = this->clients.begin(); it != this->clients.end(); )
        {
            int client_socket = it->first;
            if (FD_ISSET(client_socket, &readfds))
            {
                char buffer[1024] = {0};
                int valread = read(client_socket, buffer, sizeof(buffer));
                if (valread <= 0) // Client disconnected
                {
                    reporter.debug("Matt_daemon: Client disconnected.");
                    close(client_socket);
                    it = this->clients.erase(it);
                }
                else
                {
                    it->second += std::string(buffer, valread);
                    size_t newline_pos;
                    while ((newline_pos = it->second.find('\n')) != std::string::npos)
                    {
                        // Extract complete message
                        parse_message(it->second.substr(0, newline_pos), client_socket);

                        // Remove the processed part from the buffer
                        it->second.erase(0, newline_pos + 1);
                    }
                    ++it;
                }
            }
            else
                ++it;
        }
    }
}

Server:: Server(const Server& other)
{
    reporter.debug("Server: Copy constructor");
	*this = other;
}

Server:: ~Server()
{
    reporter.debug("Server: Destructor");
}

Server& Server:: operator=(const Server& other)
{
    reporter.debug("Server: = operand");
	if (this != &other)
		*this = other;
	return (*this);
}
