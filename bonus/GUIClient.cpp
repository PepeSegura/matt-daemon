#include "GUIclient.hpp"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const char *SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 4242;
const int LINE_HEIGHT = 20; // Height of each line of text for rendering

Display *display;
Window window;
GC gc;
int socket_fd, max_fd;

fd_set completeSet, readSet, writeSet;

bool running = true;

// Chat log and scroll position
std::vector<std::string> chat_log;
int scroll_position = 0;
Atom wmDeleteMessage; // Atom for handling window close event

void setup_x11() {
    display = XOpenDisplay(NULL);
    if (!display) {
        std::cerr << "Unable to open X display\n";
        running = false;
        return ;
    }
    
    int screen = DefaultScreen(display);
    window = XCreateSimpleWindow(display, RootWindow(display, screen),
                                 10, 10, WINDOW_WIDTH, WINDOW_HEIGHT, 1,
                                 BlackPixel(display, screen), WhitePixel(display, screen));
    
    XSelectInput(display, window, ExposureMask | KeyPressMask);
    XMapWindow(display, window);
    gc = XCreateGC(display, window, 0, NULL);

    // Register for the window close event (WM_DELETE_WINDOW)
    wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display, window, &wmDeleteMessage, 1);
}

void connect_to_server() {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        std::cerr << "Socket creation failed\n";
        running = false;
        return ;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address\n";
        running = false;
        return ;
    }

    if (connect(socket_fd, (sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection to server failed\n";
        running = false;
        return ;
    }

    FD_ZERO(&completeSet);
    FD_SET(socket_fd, &completeSet);
    max_fd = socket_fd + 1;
}

void send_message(const std::string &message) {
    std::string message_with_newline = message + "\n";
    if (send(socket_fd, message_with_newline.c_str(), message_with_newline.size(), 0) <= 0) {
        running = false;
        std::cerr << "Broken connection with server, exiting..." << std::endl;
    }
    std::cout << "Sent: " << message << std::endl;
    chat_log.push_back("Sent: " + message); // Log the sent message
}

std::string receive_message() {
    char buffer[1024];
    int bytes_received = recv(socket_fd, buffer, sizeof(buffer), 0);

    if (bytes_received <= 0) {
        running = false;
        std::cerr << "Couldn't connect with server, exiting..." << std::endl;
        return "";
    }
    std::string s(buffer, bytes_received);
    std::cout << "Received: " << s << std::endl;
    return std::string(buffer, bytes_received);
}

void update_chat_log(const std::string &text) {
    size_t pos = 0, prev_pos = 0;
    while ((pos = text.find('\n', prev_pos)) != std::string::npos) {
        chat_log.push_back("Received: " + text.substr(prev_pos, pos - prev_pos));
        prev_pos = pos + 1;
    }
}

void draw_text(const std::string &text, int x, int y) {
    XDrawString(display, window, gc, x, y, text.c_str(), text.length());
}

void draw_chat_log(std::string input_text) {
    XClearWindow(display, window);
    int start_line = std::max(0, (int)chat_log.size() - scroll_position - (WINDOW_HEIGHT / LINE_HEIGHT));
    int end_line = std::min((int)chat_log.size(), start_line + (WINDOW_HEIGHT / LINE_HEIGHT));
    draw_text(input_text, 50, 20);
    for (int i = start_line; i < end_line; ++i) {
        draw_text(chat_log[i], 50, 50 + LINE_HEIGHT * (i - start_line));
    }
    XFlush(display);
}


void cleanup() {
    // X11 resources
    if (display) {
        XFreeGC(display, gc);
        XDestroyWindow(display, window);
        XCloseDisplay(display);
    }

    // Socket connection
    if (socket_fd >= 0) {
        close(socket_fd);
    }
}

int main() {
    std::string input_text;
    std::string recv_buffer = "";

    connect_to_server();

    if (running)
        setup_x11();

    while (running) {
        readSet = writeSet = completeSet;

        if (select(max_fd, &readSet, &writeSet, NULL, NULL) == -1) {
            std::cerr << "select: error" << std::endl;
            break ;
        }

        XEvent event;
        
        if (FD_ISSET(socket_fd, &readSet)) {
            std::string response = receive_message();
            if (!response.empty()) {
                recv_buffer += response;
                update_chat_log(recv_buffer);
                recv_buffer.clear(); // Clear the buffer after processing
            }
        }

        if (XPending(display) > 0) {
            XNextEvent(display, &event);

            if (event.type == Expose) {
                draw_chat_log(input_text);
            } else if (event.type == KeyPress) {
                char buffer[1];
                KeySym keysym;
                XLookupString(&event.xkey, buffer, sizeof(buffer), &keysym, NULL);
                
                if (keysym == XK_Return && FD_ISSET(socket_fd, &writeSet)) {
                    send_message(input_text);
                    input_text.clear();
                } else if (keysym == XK_BackSpace) {
                    if (!input_text.empty()) {
                        input_text.pop_back();
                    }
                } else if (keysym == XK_Up) {
                    if (scroll_position < (int)chat_log.size() - (WINDOW_HEIGHT / LINE_HEIGHT)) {
                        scroll_position++;
                    }
                } else if (keysym == XK_Down) {
                    if (scroll_position > 0) {
                        scroll_position--;
                    }
                } else {
                    input_text += buffer[0];
                }
                draw_chat_log(input_text);
            } else if (event.type == ClientMessage) { // Handle the window close event
                if ((Atom)event.xclient.data.l[0] == wmDeleteMessage) {
                    running = false;
                }
            }
        }
    }
    cleanup();
    return 0;
}