#include "Logger.h"
#include <iostream>
#include <ctime>
#include <cstdio>
#include <iostream>
#include <sys/sysinfo.h>
#include <iomanip> // for std::fixed and std::setprecision
std::ofstream Logger::log_file;
Logger::Level Logger::threshold = Logger::Level::INFO;
std::string full_message ;
bool Logger::is_file_logging_enabled = false;

void Logger::initialize(const std::string &file_name, Level initThreshold) {
    threshold = initThreshold;
    log_file.open(file_name, std::ios::app);
    is_file_logging_enabled = true;
}

void Logger::initialize(Level initThreshold) {
    threshold = initThreshold;
    is_file_logging_enabled = false;
}

void Logger::setThreshold(Level newThreshold) {
	Logger::log(Logger::Level::INFO, "Change log level to %s",levelToString(newThreshold).c_str());
    threshold = newThreshold;
}

void Logger::log(const std::string &message, Level level) {
    log(level, "%s", message.c_str());
}

void Logger::log(Level level, const char* format, ...) {
    if (level < threshold) {
        return;
    }

    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);


    /*std::time_t now = std::time(nullptr);
    std::string time_str = std::ctime(&now);
    time_str.pop_back();*/

    struct sysinfo info;
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2) << (float)info.uptime;

       if (sysinfo(&info) == 0) {
         /*   std::cout << "System Uptime: " << info.uptime << " seconds" << std::endl;
        } else {
            std::cerr << "Failed to get system uptime" << std::endl;
        }*/

    	   //full_message = "[" << info.uptime + "] [" + levelToString(level) + "] " + buffer;
       }
    if (is_file_logging_enabled && log_file.is_open()) {
        log_file << full_message << std::endl;
    }

    std::cout << "[" << info.uptime  << "] [" << levelToString(level) << "] " << buffer << std::endl;
}

std::string Logger::levelToString(Level level) {
    switch (level) {
        case Level::TRACE:   return "TRACE";
        case Level::DEBUG:   return "DEBUG";
        case Level::INFO:    return "INFO";
        case Level::WARNING: return "WARNING";
        case Level::ERROR:   return "ERROR";
        default:             return "UNKNOWN";
    }
}
