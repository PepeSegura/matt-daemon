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

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 1800

class Ben_AFK : public Gtk::Window {
public:
    Ben_AFK();
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
    void apply_css();
    void apply_default_style(const Glib::RefPtr<Gtk::CssProvider>& css_provider);

    pthread_mutex_t msg_mtx;

};

const std::string default_css = R"(
#main_box {
    background: #f0f0f0;
    font-size: 24px;
}

#scrolled_window {
    background: #f0f0f0;
}

#message_history text{
    background: url("../assets/darkzuaje.jpg");
    font-size: 24px;
    color: #28df28;
    padding: 5px 10px;
}

#message_entry {
    font-size: 24px;
    color: #0a0a0a;
}

#send_button {
    font-size: 24px;
    color: #0f0f0f;
    background: #00a0ff;
}

#prefix_toggle {
    background: #f0f0f0;
    color: #000000;
    font-size: 24px;
}

#prefix_toggle:checked {
    background: #4CAF50;
    color: #000000;
    font-size: 24px;
}
)";