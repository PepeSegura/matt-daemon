#pragma once
#include <gtkmm.h>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <thread>
#include <pthread.h>

#define WINDOW_HEIGHT 1000
#define WINDOW_WIDTH (WINDOW_HEIGHT / 1.5)

class Ben_AFK : public Gtk::Window {
public:
    Ben_AFK(std::string css_path);
    virtual ~Ben_AFK();

protected:
    // signals
    void on_send_button_clicked();
    void on_entry_activated();
    bool on_delete_event(GdkEventAny* event) override;

    // comms
    int sock = -1;
    bool connected = false;
    fd_set all_set;

    bool connect_to_server();
    void send_message(const std::string& message);
    bool receive_messages();

    // GUI
    Gtk::Box main_box{Gtk::ORIENTATION_VERTICAL};
    Gtk::Box button_box;
    Gtk::ScrolledWindow scrolled_window;
    Gtk::TextView message_history;
    Gtk::Entry message_entry;
    Gtk::Button send_button{"Send"};
    Gtk::ToggleButton prefix_toggle{"Remote Shell"};
    void apply_css(std::string css_path);
    void apply_default_style(const Glib::RefPtr<Gtk::CssProvider>& css_provider);

    pthread_mutex_t msg_mtx;

};

const std::string default_css = R"(
#main_box {
    padding: 15px;
    border: 2px solid #ccc;
    border-radius: 10px;
    background-color: #f9f9f9;
}
#message_history {
    padding: 10px;
    border: 1px solid #ddd;
    border-radius: 5px;
    background-color: #ffffff;
    box-shadow: 0px 2px 4px rgba(0, 0, 0, 0.1);
}
#scrolled_window {
    border-radius: 10px;
    border: none;
    background-color: #f5f5f5;
    padding: 5px;
}
#message_entry {
    padding: 8px;
    border: 1px solid #ddd;
    border-radius: 5px;
    margin-top: 10px;
    margin-bottom: 10px;
    background-color: #ffffff;
}
#send_button, #prefix_toggle {
    padding: 8px 15px;
    border-radius: 5px;
    border: 1px solid #888;
    box-shadow: 0px 3px 6px rgba(0, 0, 0, 0.2);
    background-color: #e0e0e0;
    margin-right: 10px;
    transition: background-color 0.3s;
}
#send_button:hover, #prefix_toggle:hover {
    background-color: #d4d4d4;
}
#send_button:active, #prefix_toggle:active {
    box-shadow: 0px 2px 4px rgba(0, 0, 0, 0.3) inset;
}
scrollbar {
    background: transparent;
}
scrollbar trough {
    background-color: transparent;
}
scrollbar slider {
    background-color: #ccc;
    border-radius: 5px;
}

)";
