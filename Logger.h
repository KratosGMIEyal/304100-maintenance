#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <fstream>
#include <cstdarg>

class Logger {
public:
    enum class Level {
        TRACE,
        DEBUG,
        INFO,
        WARNING,
        ERROR
    };

    // Initialize the logger with a file name
    static void initialize(const std::string &file_name, Level threshold = Level::INFO);

    // Initialize the logger without a file (console-only logging)
    static void initialize(Level threshold = Level::INFO);

    // Set a new threshold log level
    static void setThreshold(Level newThreshold);

    // Log a message with a specific log level
    static void log(const std::string &message, Level level = Level::INFO);

    // Log a formatted message with a specific log level
    static void log(Level level, const char* format, ...);

private:
    static std::ofstream log_file;
    static Level threshold;
    static bool is_file_logging_enabled;
    static std::string levelToString(Level level);
};

#endif // LOGGER_H
