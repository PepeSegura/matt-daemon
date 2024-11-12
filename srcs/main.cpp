#include "Globals.hpp"

Tintin_reporter reporter("/var/log/matt_daemon/matt_daemon.log");
volatile sig_atomic_t end_program = 0;

int main(void)
{
    Matt_daemon matt;
    return (0);
}
