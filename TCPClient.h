#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <netinet/in.h>
#include <string>
#include "Config.h"
class TCPClient {
public:
    TCPClient(const std::string& ip, int port);
    ~TCPClient();

    bool connectToServer();
    void openClient();
    unsigned short calculateChecksum(unsigned short *buffer, int length);
    ssize_t sendData(int clientSocket, const char* buffer, size_t bufferSize);
    ssize_t receiveData(int clientSocket, char* buffer, size_t bufferSize);
    bool sendDataFromMemory(int clientSocket, off_t address, size_t totalSize) ;
    bool recvDataToMemory(int clientSocket, off_t address, size_t totalSize);
    void clientThread();
    void Close();
    int port;
    std::string ip;
    int clientSocket;
    void flush();
    

private:
    
    struct sockaddr_in serverAddress;
    bool isConnected;
    std::string serverHost;
};

#endif // TCPClient_H
