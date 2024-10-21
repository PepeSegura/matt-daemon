#include "Globals.hpp"

constexpr int PORT = 8080;
constexpr int MAX_CLIENTS = 3;

Server:: Server()
{
    reporter.debug("Server: Defaut constructor");
    reporter.info("PID: [" + std::to_string(getpid()) + "]");
    // while (end_program == 0)
    // {
    //     reporter.debug("Sleeping..");
    //     sleep(1);
    // }
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) // Create socket
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) // Set socket options
    {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CLIENTS) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    fd_set readfds;

    reporter.debug("Server is listening on port " + std::to_string(PORT) + " ...");
    while (end_program == 0)
    {
        // clear read_set
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int max_sd = server_fd;

        // add client sockets to the set
        for (auto it = clients.begin(); it != clients.end(); ++it)
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
            activity = select(max_sd + 1, &readfds, nullptr, nullptr, nullptr);
        } while (activity < 0 && errno == EINTR);

        if (activity < 0)
        {
            perror("Select error");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(server_fd, &readfds))
        {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) // accept new connection
            {
                perror("Accept error");
                exit(EXIT_FAILURE);
            }
            if (clients.size() == MAX_CLIENTS)
            {
                std::string error_message = "Can't accept the connection, max number of clients reached.";
                reporter.warning(error_message);
                send(new_socket, (error_message + "\n").c_str(), error_message.length() + 1, 0);
                close(new_socket);
                continue ;
            }
            // add new socket to clients map
            reporter.debug("New connection accepted.");
            clients[new_socket] = "";
        }

        // Check for activity on client sockets
        for (auto it = clients.begin(); it != clients.end(); )
        {
            int client_socket = it->first;
            if (FD_ISSET(client_socket, &readfds))
            {
                char buffer[1024] = {0};
                int valread = read(client_socket, buffer, sizeof(buffer));
                if (valread <= 0) // Client disconnected
                {
                    reporter.debug("Client disconnected.");
                    close(client_socket);
                    it = clients.erase(it);
                }
                else
                {
                    it->second += std::string(buffer, valread);
                    size_t newline_pos;
                    while ((newline_pos = it->second.find('\n')) != std::string::npos)
                    {
                        // Extract complete message
                        std::string complete_message = it->second.substr(0, newline_pos);
                        reporter.info("Received from client: " + complete_message);

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
    reporter.debug("Closing server");
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
