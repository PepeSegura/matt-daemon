#include "reporter/Tintin_reporter.hpp"

void Tintin_reporter::backup_file(void)
{
    std::time_t now = std::time(NULL);
    std::tm* local_time = std::localtime(&now);

    std::stringstream timestamp;
    timestamp  << (local_time->tm_year + 1900) << "-"
            << std::setw(2) << std::setfill('0') << (local_time->tm_mon + 1) << "-"
            << std::setw(2) << std::setfill('0') << local_time->tm_mday << "--"
            << std::setw(2) << std::setfill('0') << local_time->tm_hour << "-"
            << std::setw(2) << std::setfill('0') << local_time->tm_min << "-"
            << std::setw(2) << std::setfill('0') << local_time->tm_sec;

    std::ostringstream backup_filename;
    backup_filename << _backup_dir << "/matt_daemon--" << timestamp.str() << ".backup";

    std::ifstream src(_filename);
    std::ofstream dest(backup_filename.str(), std::ios::trunc);

    if (!src.is_open() || !dest.is_open())
    {
        std::cerr << "Error: Could not open log or backup file for copying.\n";
        return;
    }

    dest << src.rdbuf();
    src.close();
    dest.close();

    _fd = open(_filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
    if (_fd == -1)
    {
        _fd = STDERR_FILENO;
        std::cerr << "Cannot reopen: [" << _filename << "] using std::cerr as output" << std::endl;
        return;
    }
    close(_fd);
}

static bool create_directories(const std::string& path)
{
    size_t pos = 0;
    std::string current_path;

    while ((pos = path.find('/', pos)) != std::string::npos)
    {
        current_path = path.substr(0, pos++);
        if (!current_path.empty() && mkdir(current_path.c_str(), 0755) && errno != EEXIST)
        {
            std::cerr << "Error: Failed to create directory " << current_path << ": " << strerror(errno) << std::endl;
            return false;
        }
    }
    if (mkdir(path.c_str(), 0755) && errno != EEXIST)
    {
        std::cerr << "Error: Failed to create directory " << path << ": " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

void Tintin_reporter::open_file(void)
{
    if (!create_directories(_backup_dir))
        std::cerr << "Error: Failed to create backup directory structure.\n";

    _fd = open(_filename.c_str(), O_RDWR | O_CREAT | O_APPEND, 0666);
    if (_fd == -1)
    {
        _fd = STDERR_FILENO;
        std::cerr << "Cannot open: [" << _filename << "] using std::cerr as output" << std::endl;
        return ;
    }
}

Tintin_reporter:: Tintin_reporter(std::string filename) : _filename(filename)
{
    _pretty_format = true;

    size_t dir_pos = _filename.find_last_of('/');
    if (dir_pos != std::string::npos)
        _backup_dir = _filename.substr(0, dir_pos) + "/backups";
    else
        _backup_dir = "backups";

    if (!create_directories(_backup_dir))
        std::cerr << "Error: Failed to create backup directory structure.\n";

    if (_filename == "/dev/stdout")
        _fd = STDOUT_FILENO;
    else if (_filename == "/dev/stderr")
        _fd = STDERR_FILENO;
    else
    {
        open_file();
        _pretty_format = false;
    }
}

Tintin_reporter:: Tintin_reporter(const Tintin_reporter& other)
{
    *this = other;
}

Tintin_reporter:: ~Tintin_reporter()
{
    close(this->_fd);
}

Tintin_reporter& Tintin_reporter:: operator=(const Tintin_reporter& other)
{
    std::cout << "Tintin_reporter: = operand" << std::endl;
    if (this != &other)
		*this = other;
    return (*this);
}

void Tintin_reporter:: print_time(void)
{
    std::time_t now = std::time(NULL);
    std::tm* local_time = std::localtime(&now);

    std::stringstream buffer;
    buffer << "[" <<  (_pretty_format ? GREY : "")
              << std::setw(2) << std::setfill('0') << local_time->tm_mday << "/"
              << std::setw(2) << std::setfill('0') << (local_time->tm_mon + 1) << "/"
              << (local_time->tm_year + 1900) << "-"
              << std::setw(2) << std::setfill('0') << local_time->tm_hour << ":"
              << std::setw(2) << std::setfill('0') << local_time->tm_min << ":"
              << std::setw(2) << std::setfill('0') << local_time->tm_sec
              << (_pretty_format ? RESET : "")
              << "]";

    this->_log_msg += buffer.str();
}

void Tintin_reporter:: print_mode(const std::string mode)
{
    std::string color = GREEN;

    if (mode == "DEBUG")
        color = YELLOW;
    else if (mode == "INFO")
        color = GREEN;
    else if (mode == "WARNING")
        color = ORANGE;
    else if (mode == "ERROR")
        color = RED;

    std::stringstream buffer;
    buffer << " [" << (_pretty_format ? color : "") << mode << (_pretty_format ? RESET : "") << "]" << std::string(8 - mode.length(), ' ') << "- ";

    this->_log_msg += buffer.str();
}

size_t get_file_size(const std::string& filename)
{
    struct stat st;
    if (stat(filename.c_str(), &st) == 0)
        return st.st_size;
    return 0;
}

void Tintin_reporter:: print_buffer(void)
{
    if (get_file_size(_filename) + _log_msg.size() > MAX_FILE_SIZE)
        backup_file();

    open_file();
    write(this->_fd, this->_log_msg.c_str(), this->_log_msg.size());
    close(this->_fd);

    this->_log_msg = "";
}

void    Tintin_reporter:: debug(const std::string log)
{
    print_time();
    print_mode("DEBUG");

    this->_log_msg += log + "\n";
    print_buffer();
}

void    Tintin_reporter:: info(const std::string log)
{
    print_time();
    print_mode("INFO");

    this->_log_msg += log + "\n";
    print_buffer();
}

void    Tintin_reporter:: warning(const std::string log)
{
    print_time();
    print_mode("WARNING");

    this->_log_msg += log + "\n";
    print_buffer();
}

void    Tintin_reporter:: error(const std::string log)
{
    print_time();
    print_mode("ERROR");

    this->_log_msg += log + "\n";
    print_buffer();
}
