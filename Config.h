#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_FILE "/etc/config.json"
#define UDS_BIT_PATH "/tmp/uds_socket"

#include "Logger.h"

#define MAX_DATA_LENGTH 1024
#define TIMEOUT 1
#define LOG_LEVEL Logger::Level::DEBUG

#define NETWORK_INTERFACE "eth0"

//--------------------Kratos-Version-Format------------------//
struct Version {
    uint8_t major;
    uint8_t minor;
    uint8_t day;
    uint8_t month;
    uint16_t year;
};

const uint8_t PREAMBLE = 0x54 ;

const uint8_t RFSOM_RETRY = 4 ;

inline Version version = { 1, 9, 18, 11, 2024 }; // Example version information

//--------------------Socket-SBC-Settings------------------//
//const std::string ip = "10.0.1.3"; // Use the appropriate IP address
const int port = 5555; // Use the appropriate port
//--------------------Socket-RFSOM-Settings------------------//
const std::string RFSOM_ip = "10.0.2.2"; // Use the appropriate IP address
const int RFSOM_port = 7; // Use the appropriate port
const char RFSOM_interface[5] ="eth1";


#endif
