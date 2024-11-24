#include "KratosProtocol.h"
#include "Config.h"
KratosProtocol::KratosProtocol(TCPServer* server) : server(server), client(nullptr), uart(nullptr) {}
KratosProtocol::KratosProtocol(TCPClient* client) : server(nullptr), client(client), uart(nullptr) {}
KratosProtocol::KratosProtocol(UartController* uart) : server(nullptr), client(nullptr), uart(uart) {}

/*void KratosProtocol::sendFrame(Frame& frame, int client_fd) {
    frame.rawdata.clear();
    frame.preamble = PREAMBLE;
    if (frame.rawOrPacket == PACKET) {
        frame.rawdata.push_back(static_cast<uint8_t>(frame.preamble & 0xFF)); // Lower byte
        frame.rawdata.push_back(static_cast<uint8_t>((frame.preamble >> 8) & 0xFF)); // Upper byte

        auto command_ptr = reinterpret_cast<const uint8_t*>(&frame.command);
        frame.rawdata.insert(frame.rawdata.end(), command_ptr, command_ptr + sizeof(frame.command));
        auto length_ptr = reinterpret_cast<const uint8_t*>(&frame.dataLength);
        frame.rawdata.insert(frame.rawdata.end(), length_ptr, length_ptr + sizeof(frame.dataLength));

        if(frame.dataLength != 0){
        frame.rawdata.insert(frame.rawdata.end(), frame.data.begin(), frame.data.begin() + frame.dataLength);
        }
        uint16_t checksum = calculateChecksum(frame.rawdata.data(), frame.rawdata.size());
        auto checksum_ptr = reinterpret_cast<const uint8_t*>(&checksum);
        frame.rawdata.insert(frame.rawdata.end(), checksum_ptr, checksum_ptr + sizeof(checksum));

        frame.rawByteSize = sendData(client_fd, frame.rawdata.data(), frame.rawdata.size());
    } else {
        Logger::log(Logger::Level::DEBUG,"Size %d To RFSOM ",frame.rawByteSize);
        Logger::log(Logger::Level::DEBUG,"client_fd %d To RFSOM ",client_fd);
        frame.rawByteSize = sendData(client_fd, frame.rawdata.data(), frame.rawByteSize);
    }
}*/

void KratosProtocol::sendFrame(Frame& frame, int client_fd) {
    frame.rawdata.clear();
    frame.preamble = PREAMBLE;
    Logger::log(Logger::Level::ERROR, "Data size is : %d ",frame.data.size());
    //Logger::log(Logger::Level::ERROR, "Raw size is : %d ",frame.rawdata.size());
    if (frame.rawOrPacket == PACKET) {
        frame.rawdata.push_back(static_cast<uint8_t>(frame.preamble & 0xFF)); // Lower byte
        frame.rawdata.push_back(static_cast<uint8_t>((frame.preamble >> 8) & 0xFF)); // Upper byte

        auto command_ptr = reinterpret_cast<const uint8_t*>(&frame.command);
        frame.rawdata.insert(frame.rawdata.end(), command_ptr, command_ptr + sizeof(frame.command));

        auto length_ptr = reinterpret_cast<const uint8_t*>(&frame.dataLength);
        frame.rawdata.insert(frame.rawdata.end(), length_ptr, length_ptr + sizeof(frame.dataLength));

        // Check to ensure frame.dataLength is within bounds of frame.data
        if (frame.dataLength <= frame.data.size()) {
            frame.rawdata.insert(frame.rawdata.end(), frame.data.begin(), frame.data.begin() + frame.dataLength);
        } else {
            Logger::log(Logger::Level::ERROR, "Data length exceeds available data size.");
            return; // Exit function to avoid out-of-bounds access
        }

        uint16_t checksum = calculateChecksum(frame.rawdata.data(), frame.rawdata.size());

        auto checksum_ptr = reinterpret_cast<const uint8_t*>(&checksum);
        frame.rawdata.insert(frame.rawdata.end(), checksum_ptr, checksum_ptr + sizeof(checksum));


        frame.rawByteSize = sendData(client_fd, frame.rawdata.data(), frame.rawdata.size());

    } else {

        frame.rawByteSize = sendData(client_fd, frame.rawdata.data(), frame.rawByteSize);
    }
}



bool KratosProtocol::recvFrame(int client_fd, Frame& frame) {
    // Resize frame.data to the maximum expected input size
    //frame.data.resize(100);  // Adjust size according to expected input
    Logger::log(Logger::Level::DEBUG, "recvFrame start Data Size: %zu", frame.data.size());
    // Receive the preamble, command, and dataLength
    size_t headerSize = sizeof(frame.preamble) + sizeof(frame.command) + sizeof(frame.dataLength);
    size_t received = receiveData(client_fd, frame.data.data(), headerSize);

    //Logger::log(Logger::Level::DEBUG, "recvFrame middel Data Size: %zu", frame.data.size());

    if (received > 0) {
        // Adjust frame.data to the actual size received for the header
       // frame.data.resize(headerSize);

        // Extract preamble, command, and dataLength
        frame.preamble = frame.data[0];

        frame.command = *reinterpret_cast<uint16_t*>(&frame.data[2]);
        frame.dataLength = *reinterpret_cast<uint32_t*>(&frame.data[4]);

        // Resize frame.data to fit the data plus the checksum
       // frame.data.resize(frame.dataLength + sizeof(frame.checksum));

        // Receive the actual data and the checksum
        received = receiveData(client_fd, frame.data.data() , frame.dataLength + sizeof(frame.checksum));

        // Update rawByteSize to reflect total bytes received
        frame.rawByteSize = headerSize + received;
        //Logger::log(Logger::Level::DEBUG, "recvFrame end Data Size: %zu", frame.data.size());
        // Check if the received size matches the expected size
        if (received == frame.dataLength + sizeof(frame.checksum)) {
            // Successfully received all data, now extract the checksum
            frame.checksum = *reinterpret_cast<uint16_t*>(&frame.data[frame.dataLength]);

            return true;
        }
        
    }

    // If receive failed or did not match expected sizes
    //frame.data.clear();
    return false;
}

uint16_t KratosProtocol::calculateChecksum(const uint8_t* buffer, size_t length) {
    uint16_t checksum = 0;
    for (size_t i = 0; i < length; ++i) {
        checksum += buffer[i];
    }
    return checksum;
}

ssize_t KratosProtocol::sendData(int client_fd, const uint8_t* data, size_t size) {
    if (server) {
        Logger::log(Logger::Level::TRACE, "Send data to PC from SBC");
        return server->sendData(client_fd, reinterpret_cast<const char*>(data), size);
    } else if (client) {
        Logger::log(Logger::Level::TRACE, "Send data to RFSOM from SBC");
        return client->sendData(client_fd, reinterpret_cast<const char*>(data), size);
    } else if (uart) {
        Logger::log(Logger::Level::TRACE, "Send data to MCU from SBC");
        return uart->sendData(reinterpret_cast<char*>(const_cast<uint8_t*>(data)), size);
    }
    return -1;
}

ssize_t KratosProtocol::receiveData(int client_fd, uint8_t* buffer, size_t size) {
    if (server) {
        Logger::log(Logger::Level::TRACE, "Receive data from PC to SBC");
        return server->receiveData(client_fd, reinterpret_cast<char*>(buffer), size);
    } else if (client) {
        Logger::log(Logger::Level::TRACE, "Receive data from RFSOM to SBC ");
        return client->receiveData(client_fd, reinterpret_cast<char*>(buffer), size);
    } else if (uart) {
        Logger::log(Logger::Level::TRACE, "Receive data from MCU to SBC ");
        return uart->receiveData(reinterpret_cast<char*>(buffer), size); // need to fix bug on uartcontroller , 50 is max for now
    }
    return -1;
}