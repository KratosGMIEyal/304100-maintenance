#ifndef UARTCONTROLLER_H
#define UARTCONTROLLER_H

#include "Logger.h"
#include <string>

class UartController {
public:
    UartController(const std::string &portName, int baudRate);
    ~UartController();  // Destructor to handle resource cleanup

    // Initializes the UART connection
    void initialize(const std::string &portName, int baudRate);
    int getFileDescriptor();
    // Sends data through UART
    ssize_t sendData(char *buffer,size_t size);
    void flush();
    // Receives data from UART
    ssize_t receiveData(char* buffer ,ssize_t size);
    int fd;  // File descriptor for the UART port

private:
void logBuffer(char* buffer, size_t size);
    
};

#endif // UARTCONTROLLER_H
