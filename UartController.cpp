// UartController.cpp
#include "UartController.h"
#include <iostream> // Include this header for std::cerr
#include <fcntl.h>    // For file control definitions
#include <errno.h>    // For error number definitions
#include <termios.h>  // For POSIX terminal control definitions
#include <unistd.h>   // For POSIX API
#include <cstring>    // For strerror
#include <cstring>   // String function definitions
#include <sstream>
#include <iomanip>

UartController::UartController(const std::string &portName, int baudRate) {
    Logger::log("UartController created", Logger::Level::INFO);
     fd = open(portName.c_str(), O_RDWR);
    if (fd == -1) {
        Logger::log("Could not open UART port: " + portName + ". Error: " + strerror(errno), Logger::Level::ERROR);
        return;
    }

    struct termios options;
    tcgetattr(fd, &options);

    cfsetispeed(&options, baudRate);
    cfsetospeed(&options, baudRate);

    options.c_cflag |= (CLOCAL | CREAD);  // Enable the receiver and set local mode
    options.c_cflag &= ~CSIZE;            // Mask the character size bits
    options.c_cflag |= CS8;               // Select 8 data bits
    options.c_cflag &= ~PARENB;           // Disable parity bit
    options.c_cflag &= ~CSTOPB;           // Disable 2 stop bits, use 1
    options.c_cflag &= ~CRTSCTS;          // Disable hardware flow control
    options.c_cflag |= CREAD | CLOCAL; // turn on READ & ignore ctrl lines

    //options.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
    //options.c_cc[VMIN] = 0;
    options.c_lflag = 0;

    options.c_cc[VMIN] = 0;            // dont Read blocks
    options.c_cc[VTIME] = 1;           // 0.5 seconds read timeout

    // Apply the settings
    //tcsetattr(fd, TCSANOW, &options);

    // Make raw
    cfmakeraw(&options);

    // Flush Port, then applies attributes
    tcflush(fd, TCIFLUSH);
    if (tcsetattr(fd, TCSANOW, &options) != 0) {
        Logger::log(Logger::Level::ERROR, "Error reading: %s", strerror(errno));
    }

    Logger::log("Initialized UART on port: " + portName + " with baud rate: " + std::to_string(baudRate), Logger::Level::INFO);
}

UartController::~UartController() {
    if (fd != -1) {
        close(fd);
    }
}

/*void UartController::initialize(const std::string &portName, int baudRate) {
    fd = open(portName.c_str(), O_RDWR);
    if (fd == -1) {
        Logger::log("Could not open UART port: " + portName + ". Error: " + strerror(errno), Logger::Level::ERROR);
        return;
    }

    struct termios options;
    tcgetattr(fd, &options);
    cfsetispeed(&options, baudRate);
    cfsetospeed(&options, baudRate);

    options.c_cflag |= (CLOCAL | CREAD);  // Enable the receiver and set local mode
    options.c_cflag &= ~CSIZE;            // Mask the character size bits
    options.c_cflag |= CS8;               // Select 8 data bits
    options.c_cflag &= ~PARENB;           // Disable parity bit
    options.c_cflag &= ~CSTOPB;           // Disable 2 stop bits, use 1
    options.c_cflag &= ~CRTSCTS;          // Disable hardware flow control

    // Apply the settings
    tcsetattr(fd, TCSANOW, &options);

    Logger::log("Initialized UART on port: " + portName + " with baud rate: " + std::to_string(baudRate), Logger::Level::INFO);
}*/
int UartController::getFileDescriptor()
{
    return fd;
}

void UartController::flush() {
    if (fd == -1) {
        Logger::log("UART port not initialized", Logger::Level::ERROR);
        return;
    }

    if (tcflush(fd, TCIOFLUSH) != 0) {
        Logger::log("Failed to flush UART. Error: " + std::string(strerror(errno)), Logger::Level::ERROR);
    } else {
        Logger::log("Successfully flushed UART", Logger::Level::TRACE);
    }
}

ssize_t UartController::sendData(char *buffer,size_t size) {
    if (fd == -1) {
        Logger::log("UART port not initialized", Logger::Level::ERROR);
        return 0;
    }

    logBuffer(buffer,size);
  /*  for (size_t i = 0; i < size; i++)
    {
        printf("0x%X ",buffer[i]);
    }
    printf("\n");*/

    ssize_t count = 0;
    /*for (size_t i = 0; i < size; i++)
    {
        count += write(fd, (buffer+i), 1);
    }*/
    count = write(fd, buffer, size);
    
    if (count < 0) {
        Logger::log("Failed to write to UART. Error: " + std::string(strerror(errno)), Logger::Level::ERROR);
    } else {
        Logger::log(Logger::Level::TRACE,"count Sent data through UART: %d" ,count);
    }

    return count;
}

ssize_t UartController::receiveData(char* buffer ,ssize_t size) {
   // memset(buffer, 0, size);
    if (fd == -1) {
        Logger::log("UART port not initialized", Logger::Level::ERROR);
        return 0;
    }
    ssize_t count = 0;
   /* for (size_t i = 0; i < size; i++)
    {
        //count += read(fd, (buffer+i), 1);
       
       // Logger::log(Logger::Level::WARNING,"0x%X ",buffer[i]);
       // printf("0x%X ",buffer[i]);
    }*/
    count = read(fd, buffer, size);
    logBuffer(buffer,count);
   /* for (size_t i = 0; i < size; i++)
    {
        printf("0x%X ",buffer[i]);
    }
    
    printf("\n");*/

    if (count < 0) {
        Logger::log("Failed to read from UART. Error: " + std::string(strerror(errno)), Logger::Level::ERROR);
        return count;
    } else {
        Logger::log(Logger::Level::TRACE,"Received data through UART: %d ", count);
        return count;
    }
}

void UartController::logBuffer(char* buffer, size_t size)
{
    std::stringstream ss;
    ss << "UART DATA :" ;
    for (size_t i = 0; i < size; ++i)
    {
        ss << "0x" << std::uppercase << std::hex << static_cast<int>(buffer[i]) << " ";
    }
    Logger::log(ss.str().c_str(), Logger::Level::TRACE);
}