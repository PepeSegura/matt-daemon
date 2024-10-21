#include "Globals.hpp"

constexpr int PORT = 8080;
constexpr int MAX_CLIENTS = 3;

Server:: Server()
{
    reporter.debug("Server: Defaut constructor");
    reporter.info("PID: [" + std::to_string(getpid()) + "]");

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("Setsockopt failed");
        exit(EXIT_FAILURE);
    }

    // Define the socket address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, MAX_CLIENTS) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    std::vector<int> clients; // To hold client socket descriptors
    fd_set readfds;

    reporter.debug("Server is listening on port " + std::to_string(PORT) + "...");
    while (end_program == 0)
    {
        // Clear the set
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        int max_sd = server_fd;

        // Add client sockets to the set
        for (int client : clients)
        {
            FD_SET(client, &readfds);
            if (client > max_sd)
                max_sd = client;
        }

        // Wait for activity on the sockets
        int activity = select(max_sd + 1, &readfds, nullptr, nullptr, nullptr);
        if (activity < 0)
        {
            perror("Select error");
            exit(EXIT_FAILURE);
        }

        // Check if there's an incoming connection on the server socket
        if (FD_ISSET(server_fd, &readfds))
        {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
            {
                perror("Accept error");
                exit(EXIT_FAILURE);
            }
            // Add new socket to clients
            if (clients.size() == MAX_CLIENTS)
            {
                std::string error_message = "Can't accept the connection, max number of clients reached.";
                reporter.warning(error_message);
                send(new_socket, (error_message + "\n").c_str(), error_message.length() + 1, 0);
                close(new_socket);
                continue ;
            }
            reporter.debug("New connection accepted.");
            clients.push_back(new_socket);
        }

        // Check for activity on client sockets
        for (size_t i = 0; i < clients.size(); ++i)
        {
            int client_socket = clients[i];
            if (FD_ISSET(client_socket, &readfds))
            {
                char buffer[131072] = {0};
                int valread = read(client_socket, buffer, sizeof(buffer));
                if (valread <= 0)
                {
                    // Client disconnected
                    reporter.debug("Client disconnected.");
                    close(client_socket);
                    clients.erase(clients.begin() + i);
                    --i; // Adjust index after erasing
                }
                else
                {
                    std::string message(buffer);
                    if (!message.empty() && message.back() == '\n')
                        message.pop_back();
                    reporter.info("Received: " + message);
                }
            }
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
