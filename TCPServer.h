#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <netinet/in.h>
#include <string>

class TCPServer {
public:
    TCPServer(const std::string& ip, int port);
    ~TCPServer();

    void init(const std::string &ip, int port);
    bool start();
    void stop();
    void Close(int fd);
    void openServer();
    int acceptConnection();
    ssize_t sendData(int clientSocket, const char* buffer, size_t bufferSize);
    ssize_t receiveData(int clientSocket, char* buffer, size_t bufferSize);
    bool sendDataFromMemory(int clientSocket, off_t address, size_t totalSize) ;
    bool recvDataToMemory(int clientSocket, off_t address, size_t totalSize);
    int port;
    std::string ip;
    static std::string getIPAddress(const std::string& interface);

private:
    int serverSocket;
    struct sockaddr_in serverAddress;
};

#endif // TCPSERVER_H
