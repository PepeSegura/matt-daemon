#include "GUIclient.hpp"
    
int main() {
    GUIClient client;

    return 0;
}

GUIClient::GUIClient() {
    std::string input_text;
    std::string recv_buffer = "";
    display = NULL;

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
                recv_buffer.clear();
                draw_chat_log(input_text);
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
                } else if (isprint(buffer[0])) {
                    input_text += buffer[0];
                }
                draw_chat_log(input_text);
            } else if (event.type == ClientMessage) {
                if ((Atom)event.xclient.data.l[0] == wmDeleteMessage) {
                    running = false;
                }
            }
        }
    }
}

GUIClient::~GUIClient() { cleanup(); }

void GUIClient::setup_x11() {
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

    wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False); //register window close event
    XSetWMProtocols(display, window, &wmDeleteMessage, 1);
}

void GUIClient::connect_to_server() {
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

void GUIClient::send_message(const std::string &message) {
    std::string message_with_newline = message + "\n";
    if (send(socket_fd, message_with_newline.c_str(), message_with_newline.size(), 0) <= 0) {
        running = false;
        std::cerr << "Broken connection with server, exiting..." << std::endl;
    }
    std::cout << "Sent: " << message << std::endl;
    chat_log.push_back("Sent: " + message);
}

std::string GUIClient::receive_message() {
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

void GUIClient::update_chat_log(const std::string &text) {
    size_t pos = 0, prev_pos = 0;
    while ((pos = text.find('\n', prev_pos)) != std::string::npos) {
        chat_log.push_back("Received: " + text.substr(prev_pos, pos - prev_pos));
        prev_pos = pos + 1;
    }
}

void GUIClient::draw_text(const std::string &text, int x, int y) {
    XDrawString(display, window, gc, x, y, text.c_str(), text.length());
}

void GUIClient::draw_chat_log(std::string input_text) {
    XClearWindow(display, window);
    int start_line = std::max(0, (int)chat_log.size() - scroll_position - (WINDOW_HEIGHT / LINE_HEIGHT));
    int end_line = std::min((int)chat_log.size(), start_line + (WINDOW_HEIGHT / LINE_HEIGHT));
    draw_text(input_text, 50, 20);
    for (int i = start_line; i < end_line; ++i) {
        draw_text(chat_log[i], 50, 50 + LINE_HEIGHT * (i - start_line));
    }
    XFlush(display);
}


void GUIClient::cleanup() {
    if (display) {
        XFreeGC(display, gc);
        XDestroyWindow(display, window);
        XCloseDisplay(display);
    }
    
    if (socket_fd >= 0) {
        close(socket_fd);
    }
}
