#include "TCPClient.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/ip_icmp.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <vector>
#include "MemoryAccess.h"
#include "Logger.h"
#include <thread> // For std::thread
#include <atomic> // For std::atomic
#include <chrono>
#include <future>
#include <termios.h>

TCPClient::TCPClient(const std::string& host, int port) 
    : clientSocket(-1), isConnected(false), serverHost(host), port(port) {
    openClient();
}

TCPClient::~TCPClient() {
    if (isConnected) {
        close(clientSocket);
    }
}

void TCPClient::clientThread() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

void TCPClient::openClient() {
    if (!connectToServer()) {
        Logger::log(Logger::Level::ERROR, "Failed to start client");
        Close();
    } else {
        Logger::log(Logger::Level::INFO, "Client started");
    }
}

bool TCPClient::connectToServer() {
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) {
        Logger::log(Logger::Level::ERROR, "Could not create socket");
        return false;
    }

    if (setsockopt(clientSocket, SOL_SOCKET, SO_BINDTODEVICE, RFSOM_interface, strlen(RFSOM_interface)) < 0) {
        Logger::log(Logger::Level::ERROR, "Error binding to interface");
        close(clientSocket);
        return false;
    }

    sockaddr_in server;
    server.sin_addr.s_addr = inet_addr(serverHost.c_str());
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if (connect(clientSocket, (struct sockaddr *)&server, sizeof(server)) < 0) {
        Logger::log(Logger::Level::ERROR, "Connect failed. Error");
        return false;
    }

    isConnected = true;
    Logger::log(Logger::Level::INFO, "Connect success");
    return true;
}

ssize_t TCPClient::sendData(int clientSocket, const char* buffer, size_t bufferSize) {
    ssize_t result = send(clientSocket, buffer, bufferSize, 0);
    if (result == -1) {
        std::cerr << "Send error: " << strerror(errno) << "\n";
    }
    return result;
}

void TCPClient::Close() {
    close(clientSocket);
}

ssize_t TCPClient::receiveData(int clientSocket, char* buffer, size_t bufferSize) {
    struct timeval timeout;
    timeout.tv_sec = 5; // טיימאוט של 5 שניות
    timeout.tv_usec = 0;

    if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
       // Logger::log(Logger::Level::ERROR, "Failed to set receive timeout: %s", strerror(errno));
        return -1;
    }

    return recv(clientSocket, buffer, bufferSize, MSG_WAITALL);
}

bool TCPClient::sendDataFromMemory(int clientSocket, off_t address, size_t totalSize) {
    MemoryAccess memAccess;
    char* data = (char*)memAccess.read_physical_mem(address, totalSize);
    if (data) {
        ssize_t result = sendData(clientSocket, data, totalSize);
        Logger::log(Logger::Level::DEBUG, "TCP send %zd bytes", result);
    } else {
        Logger::log(Logger::Level::ERROR, "Failed to read data from physical memory");
    }
    return true;
}

bool TCPClient::recvDataToMemory(int clientSocket, off_t address, size_t totalSize) {
    MemoryAccess memAccess;
    char* data = memAccess.read_physical_mem(address, totalSize);
    char* mallocData = (char*)malloc(totalSize);

    ssize_t result = receiveData(clientSocket, mallocData, totalSize);
    Logger::log(Logger::Level::DEBUG, "TCP receive %zd bytes", result);

    for (size_t i = 0; i < totalSize; i++) {
        data[i] = mallocData[i];
    }
    free(mallocData);

    return true;
}

void TCPClient::flush() {
    if (clientSocket == -1) {
        Logger::log(Logger::Level::ERROR, "Socket is not open, cannot flush");
        return;
    }

    // Ensure that all data has been sent
    int result;
    do {
        result = send(clientSocket, nullptr, 0, MSG_NOSIGNAL);
    } while (result == -1 && errno == EAGAIN);

    if (result == -1) {
        Logger::log(Logger::Level::ERROR, "Flush failed: %s", strerror(errno));
    } else {
        Logger::log(Logger::Level::TRACE, "Flush successful");
    }
}
