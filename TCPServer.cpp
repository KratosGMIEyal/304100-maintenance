#include "TCPServer.h"
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <sys/ioctl.h>  // Required for ioctl and SIOCGIFADDR
#include <vector>
#include <cstdio> // For snprintf (if needed for logging purposes)
#include <net/if.h>
#include "MemoryAccess.h"
#include "Logger.h"

int opt = 1;
const int RECEIVE_TIMEOUT_SEC = 5;

TCPServer::TCPServer(const std::string& ip, int port) :
        ip(ip), port(port), serverSocket(-1) {
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr);
    serverAddress.sin_port = htons(port);
    openServer();
}

TCPServer::~TCPServer() {
    stop();
}

void TCPServer::init(const std::string& ip, int port) {
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr);
    serverAddress.sin_port = htons(port);
}

bool TCPServer::start() {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        return false;
    }
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        return false;
    }
    if (listen(serverSocket, 5) < 0) {
        return false;
    }
    return true;
}

void TCPServer::Close(int fd) {
    close(fd);
}

void TCPServer::stop() {
    if (close(serverSocket) == -1) {
        std::cerr << "Failed to close socket.\n";
    }
    std::cout << "Socket successfully closed.\n";
}

void TCPServer::openServer() {
    if (!start()) {
        Logger::log(Logger::Level::ERROR, "Failed to start server on IP %s & Port: %d",ip.c_str(), port);
    } else {
        Logger::log(Logger::Level::INFO, "Server started on IP %s & Port: %d", ip.c_str(), port);
    }
}

int TCPServer::acceptConnection() {
    int clientSocket = accept(serverSocket, nullptr, nullptr);
    return clientSocket;
}

ssize_t TCPServer::sendData(int clientSocket, const char* buffer, size_t bufferSize) {
    ssize_t result = send(clientSocket, buffer, bufferSize, 0);
    if (result == -1) {
        Logger::log(Logger::Level::ERROR, "Send error: %s", strerror(errno));
    }
    return result;
}

ssize_t TCPServer::receiveData(int clientSocket, char* buffer, size_t bufferSize) {
    /*struct timeval timeout;
    timeout.tv_sec = RECEIVE_TIMEOUT_SEC;
    timeout.tv_usec = 0;

    if (setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
       // Logger::log(Logger::Level::ERROR, "Failed to set receive timeout: %s", strerror(errno));
        return -1;
    }*/

    return recv(clientSocket, buffer, bufferSize, MSG_WAITALL);
}

bool TCPServer::sendDataFromMemory(int clientSocket, off_t address, size_t totalSize) {
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

bool TCPServer::recvDataToMemory(int clientSocket, off_t address, size_t totalSize) {
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

std::string TCPServer::getIPAddress(const std::string& interface) {
    int fd;
    struct ifreq ifr;

    // Create a socket to perform network interface operations
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) {
        Logger::log(Logger::Level::ERROR, "Failed to create socket for retrieving IP address of interface: %s", interface.c_str());
        return "";
    }

    // Specify the interface to get the IP address
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ-1);

    // Perform the IOCTL system call to get the IP address
    if (ioctl(fd, SIOCGIFADDR, &ifr) == -1) {
        Logger::log(Logger::Level::ERROR, "Failed to retrieve IP address for interface: %s", interface.c_str());
        close(fd);
        return "";
    }

    close(fd);

    // Convert the IP address from binary to a human-readable string
    char ipAddress[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &(((struct sockaddr_in*)&ifr.ifr_addr)->sin_addr), ipAddress, INET_ADDRSTRLEN) == nullptr) {
        Logger::log(Logger::Level::ERROR, "Failed to convert IP address to string for interface: %s", interface.c_str());
        return "";
    }

    Logger::log(Logger::Level::DEBUG, "Successfully retrieved IP address %s for interface: %s", ipAddress, interface.c_str());
    return std::string(ipAddress);
}
