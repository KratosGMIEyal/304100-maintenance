#ifndef KRATOSPROTOCOL_H
#define KRATOSPROTOCOL_H

#define RAW true
#define PACKET false

#include <vector>
#include <cstdint>
#include "TCPServer.h"
#include "TCPClient.h"
#include "UartController.h"

struct Frame {
    uint16_t preamble;
    uint16_t command;
    uint32_t dataLength;
    std::vector<uint8_t> data;
    uint16_t checksum;
    std::vector<uint8_t> rawdata;
    ssize_t rawByteSize;
    bool rawOrPacket;

    Frame(size_t data_size = 5000000) : data(data_size), rawdata(data_size) {} // Initialize data with a fixed size
};

class KratosProtocol {
private:
    TCPServer* server;
    TCPClient* client;
    UartController* uart;

public:
    KratosProtocol(TCPServer* server);
    KratosProtocol(TCPClient* client);
    KratosProtocol(UartController* uart);

    void sendFrame(Frame& frame, int client_fd);
    bool recvFrame(int client_fd, Frame& frame);

private:
    uint16_t calculateChecksum(const uint8_t* buffer, size_t length);
    ssize_t sendData(int client_fd, const uint8_t* data, size_t size);
    ssize_t receiveData(int client_fd, uint8_t* buffer, size_t size);
};

#endif // KRATOSPROTOCOL_H
