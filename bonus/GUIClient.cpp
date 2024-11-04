#include "GUIclient.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" //for loading images

int main() {
    GUIClient client;

    return 0;
}

GUIClient::GUIClient() {
    std::string input_text;
    std::string recv_buffer = "";
    display = NULL;
    running = true;

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
                } else if (keysym == XK_Escape) {
                    running = false;
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

    background = load_background_image(BACKGROUND_PATH);
    if (!background) {
        std::cerr << "Unable to load background image, exiting...\n";
        running = false;
        return ;
    }
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

void GUIClient::update_chat_log(std::string &text) {
    size_t pos = 0;
    while ((pos = text.find('\n')) != std::string::npos) {
        chat_log.push_back("Received: " + text.substr(0, pos));
        text = text.substr(pos + 1);
    }
}

void GUIClient::draw_text(const std::string &text, int x, int y) {
    XDrawString(display, window, gc, x, y, text.c_str(), text.length());
}

void GUIClient::draw_chat_log(std::string input_text) {
    XClearWindow(display, window);
    draw_background(background);
    XSetForeground(display, gc, 0xffffff);
    int start_line = std::max(0, (int)chat_log.size() - scroll_position - (int)((WINDOW_HEIGHT / LINE_HEIGHT) * 0.8));
    int end_line = std::min((int)chat_log.size(), start_line + (int)((WINDOW_HEIGHT / LINE_HEIGHT) * 0.8));
    draw_text("Input:", 50, 790);
    draw_text(input_text, 50, 820);
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

    if (background) {
        delete[] background->data;
        background->data = nullptr;
        XDestroyImage(background);
    }
}

XImage *GUIClient::load_background_image(const char *filepath) {
    int width, height, channels;
    unsigned char *image_data = stbi_load(filepath, &width, &height, &channels, 4);  // Load image with 4 channels (RGBA)
    
    if (!image_data) {
        std::cerr << "Failed to load image: " << filepath << "\n";
        return nullptr;
    }

    // Convert from RGBA to BGRA
    for (int i = 0; i < width * height; ++i) {
        unsigned char* pixel = image_data + i * 4;

        // Swap the Red and Blue channels
        std::swap(pixel[0], pixel[2]);
    }

    // Allocate memory for the rotated image (rotated width and height)
    unsigned char* rotated_data = new unsigned char[width * height * 4];  // 4 bytes per pixel (BGRA)

    // Rotate by 90 degrees counterclockwise
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            // Source pixel in original image
            unsigned char* src_pixel = image_data + ((y * width + x) * 4);
            
            // Destination pixel in rotated image
            int rotated_x = height - y - 1;
            int rotated_y = x;
            unsigned char* dst_pixel = rotated_data + ((rotated_y * height + rotated_x) * 4);

            // Copy pixel data
            std::memcpy(dst_pixel, src_pixel, 4);
        }
    }

    // Free the original image data
    stbi_image_free(image_data);

    // Create an XImage from the rotated data
    XImage* ximage = XCreateImage(display, DefaultVisual(display, 0), DefaultDepth(display, 0), ZPixmap, 0,
                                  (char*)rotated_data, height, width, 32, 0);

    if (!ximage) {
        std::cerr << "Failed to create XImage\n";
        delete[] rotated_data;  // Free rotated data if XImage creation failed
        return nullptr;
    }

    return ximage;  // Note: rotated_data is owned by ximage now
}

void GUIClient::draw_background(XImage *ximage) {
    XPutImage(display, window, gc, ximage, 0, 0, 0, 0, ximage->width, ximage->height);
    XFlush(display);  // Flush the display to ensure the image is shown
}
