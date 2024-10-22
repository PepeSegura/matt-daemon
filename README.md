Check bugs

deamon stops writing in log file after this steps
1. ./Matt_daemon // opening daemon
2. rm hola.log
3. ./Matt_daemon // error already one daemon running
4. nc localhost 4242

I think that is because the first daemon has an fd linked to hola.log, but hola.log is not even open in that process
