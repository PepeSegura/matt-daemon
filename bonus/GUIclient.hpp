#pragma once
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>

class GUIClient {
    public:
        GUIClient();
        ~GUIClient();

    private:
        /* PRIVATE VARS/CONSTS */
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

        /* PRIVATE METHODS */
        void        setup_x11();
        void        cleanup();
        void        connect_to_server();
        void        send_message(const std::string &message);
        std::string receive_message();
        void        update_chat_log(const std::string &text);
        void        draw_text(const std::string &text, int x, int y);
        void        draw_chat_log(std::string input_text);

};