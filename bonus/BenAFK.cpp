#include "BenAFK.hpp"

Ben_AFK::Ben_AFK(): button_box(Gtk::ORIENTATION_HORIZONTAL) {
    set_title("Ben_AFK");
    set_default_size(600, 900);

    // msg history
    message_history.set_editable(false);
    message_history.set_wrap_mode(Gtk::WRAP_WORD);

    // scrolled msg window
    scrolled_window.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolled_window.add(message_history);
    scrolled_window.set_min_content_height(200);
    scrolled_window.set_name("main_box");

    // set up main layout
    main_box.set_name("main_box");
    message_history.set_name("message_history");
    message_entry.set_name("message_entry");
    send_button.set_name("send_button");
    prefix_toggle.set_name("prefix_toggle");
    main_box.pack_start(scrolled_window, Gtk::PACK_EXPAND_WIDGET);
    main_box.pack_start(message_entry, Gtk::PACK_SHRINK);
    button_box.pack_start(send_button, Gtk::PACK_SHRINK);
    button_box.pack_start(prefix_toggle, Gtk::PACK_SHRINK);
    main_box.pack_start(button_box, Gtk::PACK_SHRINK);

    add(main_box);

    apply_css();

    // connect signals
    send_button.signal_clicked().connect(sigc::mem_fun(*this, &Ben_AFK::on_send_button_clicked));
    message_entry.signal_activate().connect(sigc::mem_fun(*this, &Ben_AFK::on_entry_activated));

    connected = connect_to_server();
    if (connected) {
        receive_thread = std::thread(&Ben_AFK::receive_messages, this);
    } else {
        exit(1);
    }

    show_all_children();
}

Ben_AFK::~Ben_AFK() {
    if (connected) {
        ::close(sock);
    }
    if (receive_thread.joinable()) {
        receive_thread.detach();
    }
}

void Ben_AFK::apply_css() {
    auto css_provider = Gtk::CssProvider::create();

    const std::string css_path = "bonus/Ben_AFK.css";
    try {
        if (!css_provider->load_from_path(css_path)) {
            std::cerr << "Error: Failed to load CSS from " << css_path << std::endl;
            apply_default_style(css_provider);
        }
    } catch (const Glib::Error& ex) {
        std::cerr << "CSS loading error: " << ex.what() << std::endl;
        apply_default_style(css_provider);
    }

    auto screen = Gdk::Screen::get_default();
    auto context = main_box.get_style_context();
    context->add_provider_for_screen(screen, css_provider, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

void Ben_AFK::apply_default_style(const Glib::RefPtr<Gtk::CssProvider>& css_provider) {
    css_provider->load_from_data(
        default_css
    );
}

bool Ben_AFK::connect_to_server() {
    const char* server_ip = "127.0.0.1";
    const int server_port = 4242;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return false;
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
        std::cerr << "Invalid address" << std::endl;
        return false;
    }

    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        return false;
    }

    std::cout << "Connected to " << server_ip << ":" << server_port << " through fd " << sock << std::endl;
    FD_ZERO(&all_set);
    FD_SET(sock, &all_set);
    return true;
}

void Ben_AFK::on_send_button_clicked() {
    std::string message = message_entry.get_text();

    if (prefix_toggle.get_active()) {
        message = "sh: " + message;
    }

    message += '\n';

    send_message(message);

    message_entry.set_text("");
}

void Ben_AFK::on_entry_activated() {
    on_send_button_clicked();
}

bool Ben_AFK::on_delete_event(GdkEventAny* event) {
    should_exit = true;
    if (connected) {
        ::close(sock);
        connected = false;
    }

    return Gtk::Window::on_delete_event(event);
}

void Ben_AFK::send_message(const std::string& message) {
    fd_set write_set = all_set;

    select(sock + 1, NULL, &write_set, NULL,NULL);

    if (connected && !message.empty() && FD_ISSET(sock, &write_set)) {
        //printf("Sending: (%s) through fd %i, send returned %li\n", message.c_str(), sock, );
        send(sock, message.c_str(), message.size(), 0);

        //update msg history
        Glib::RefPtr<Gtk::TextBuffer> msg_buffer = message_history.get_buffer();
        msg_buffer->insert(msg_buffer->end(), "Ben_AFK: " + message);

        auto mark = msg_buffer->create_mark(msg_buffer->end());
        message_history.scroll_to(mark);
    }
}

void Ben_AFK::receive_messages() {
    fd_set read_set;
    std::string big_buffer = "";
    char buffer[1024];
    while (!should_exit) {
        read_set = all_set;
        select(sock + 1, &read_set, NULL, NULL,NULL);
        if (FD_ISSET(sock, &read_set)) {
            ssize_t bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received > 0) {
                buffer[bytes_received] = '\0';
                //printf("Received: (%s)\n", buffer);
                big_buffer += buffer;

                size_t pos = 0;
                while ((pos = big_buffer.find('\n')) != std::string::npos) {
                    std::string server_message = "Matt_daemon: " + big_buffer.substr(0, pos) + "\n";

                    // update msg history in main thread
                    Glib::RefPtr<Gtk::TextBuffer> msg_buffer = message_history.get_buffer();
                    msg_buffer->insert(msg_buffer->end(), server_message);
                    big_buffer = big_buffer.substr(pos + 1);

                    auto mark = msg_buffer->create_mark(msg_buffer->end());
                    message_history.scroll_to(mark);
                }
            } else {
                break ;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    auto app = Gtk::Application::create(argc, argv, "com.MatiasEndomoniao.BenAFK");

    Ben_AFK window;

    app->run(window);

    return 0;
}